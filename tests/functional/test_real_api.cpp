#include <gtest/gtest.h>
#include <tpp/application.h>
#include <tpp/eventsub_client.h>
#include <tpp/https_client.h>
#include <tpp/session.h>

#include <atomic>
#include <cctype>
#include <chrono>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

namespace {

uint16_t next_test_port() {
  static std::atomic<uint16_t> port {23000};
  return port.fetch_add(1);
}

struct test_credentials {
  std::string client_id;
  std::string client_secret;
  bool        loaded {false};
};

std::string trim(std::string s) {
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
    s.erase(s.begin());
  }
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
    s.pop_back();
  }
  return s;
}

/* Real Twitch application credentials, read from the TEST_SECTRETS file at
 * the repo root (path baked in via -DTPP_TEST_SECRETS_PATH by
 * tests/meson.build). Never committed to version control; if it's missing
 * every test below skips itself rather than failing, so this suite still
 * passes in an environment without secrets or network access. */
test_credentials load_test_credentials() {
  test_credentials creds;
#ifdef TPP_TEST_SECRETS_PATH
  std::ifstream file(TPP_TEST_SECRETS_PATH);
  if (!file) {
    return creds;
  }
  std::string line;
  while (std::getline(file, line)) {
    auto eq = line.find('=');
    if (eq == std::string::npos) {
      continue;
    }
    std::string key   = trim(line.substr(0, eq));
    std::string value = trim(line.substr(eq + 1));
    if (key == "client_id") {
      creds.client_id = value;
    } else if (key == "client_secret") {
      creds.client_secret = value;
    }
  }
#endif
  creds.loaded = !creds.client_id.empty() && !creds.client_secret.empty();
  return creds;
}

template<typename Pred>
bool wait_for(Pred                      pred,
              std::chrono::milliseconds timeout = std::chrono::seconds(15)) {
  auto start = std::chrono::steady_clock::now();
  while (!pred()) {
    if (std::chrono::steady_clock::now() - start > timeout) {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return true;
}

struct http_result {
  uint16_t    status {0};
  std::string body;
};

http_result do_request(tpp::application &app, const std::string &host,
                       const std::string &path, const std::string &verb,
                       const std::string       &body,
                       const tpp::http_headers &headers) {
  http_result       result;
  std::atomic<bool> done {false};

  auto client = std::make_unique<tpp::https_client>(
      &app, host, 443, path, verb, body, headers, false, 10, "1.1",
      [&](tpp::https_client *c) {
        result.status = c->get_status();
        result.body   = c->get_content();
        done          = true;
      });

  wait_for([&] { return done.load(); });
  return result;
}

/* Fetches a real app access token via the Client Credentials Grant,
 * skipping the calling test if that fails - the token tests below build on
 * top of this. */
bool fetch_app_token(tpp::application &app, std::string &token,
                     uint64_t &expires_in) {
  std::atomic<bool> done {false};
  bool              ok = false;
  app.get_app_access_token(
      [&](bool success, const std::string &tok, uint64_t exp) {
        ok         = success;
        token      = tok;
        expires_in = exp;
        done       = true;
      });
  wait_for([&] { return done.load(); });
  return ok;
}

}// namespace

class RealApiTest : public ::testing::Test {
 protected:
  test_credentials creds = load_test_credentials();

  void SetUp() override {
    if (!creds.loaded) {
      GTEST_SKIP() << "TEST_SECTRETS not found at build time; skipping tests "
                      "against the real Twitch API";
    }
  }
};

TEST_F(RealApiTest, ClientCredentialsGrantReturnsValidToken) {
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t    expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));
  EXPECT_FALSE(token.empty());
  EXPECT_GT(expires_in, 0u);

  app.stop();
}

TEST_F(RealApiTest, ClientCredentialsGrantWithBadSecretFails) {
  tpp::application app(creds.client_id, next_test_port(),
                       "definitely_not_the_secret");
  app.start(false);

  std::string token;
  uint64_t    expires_in = 0;
  EXPECT_FALSE(fetch_app_token(app, token, expires_in));
  EXPECT_TRUE(token.empty());

  app.stop();
}

TEST_F(RealApiTest, RefreshUserTokenWithFakeTokenFails) {
  /* We don't have a real user refresh token to test rotation with (getting
   * one needs an interactive browser round trip through a grant type
   * tpp::application doesn't implement yet), but this still exercises the
   * real request path - grant_type=refresh_token against the real token
   * endpoint - and confirms Twitch's rejection of a bogus refresh token is
   * correctly reported as failure. */
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  std::atomic<bool> done {false};
  bool              ok = true;
  app.refresh_user_token(
      "not_a_real_refresh_token",
      [&](bool success, const std::string &, const std::string &, uint64_t) {
        ok   = success;
        done = true;
      });

  ASSERT_TRUE(wait_for([&] { return done.load(); }));
  EXPECT_FALSE(ok);

  app.stop();
}

TEST_F(RealApiTest, AppAccessTokenValidates) {
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t    expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));

  /* Twitch's /oauth2/validate endpoint specifically wants the "OAuth"
   * auth scheme rather than "Bearer". */
  tpp::http_headers headers {
      {"Authorization", "OAuth " + token},
  };
  http_result result =
      do_request(app, "id.twitch.tv", "/oauth2/validate", "GET", "", headers);

  EXPECT_EQ(result.status, 200);
  EXPECT_NE(result.body.find(creds.client_id), std::string::npos);

  app.stop();
}

TEST_F(RealApiTest, HelixGamesLookupWithAppToken) {
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t    expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));

  tpp::http_headers headers {
      {"Authorization", "Bearer " + token},
      {"Client-Id",     creds.client_id  },
  };
  http_result result =
      do_request(app, "api.twitch.tv", "/helix/games?name=Just%20Chatting",
                 "GET", "", headers);

  EXPECT_EQ(result.status, 200);
  EXPECT_NE(result.body.find("\"data\""), std::string::npos);

  app.stop();
}

TEST_F(RealApiTest, EventSubWebSocketConnectsAndWelcomes) {
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  std::mutex        mutex;
  std::atomic<bool> welcomed {false};
  std::string       session_id;

  auto client        = std::make_unique<tpp::eventsub_client>(&app);
  client->on_welcome = [&](const std::string &id) {
    std::lock_guard<std::mutex> lock(mutex);
    session_id = id;
    welcomed   = true;
  };

  ASSERT_TRUE(wait_for([&] { return welcomed.load(); }));
  {
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_FALSE(session_id.empty());
  }

  /* Connections the caller constructs directly (rather than ones owned by
   * a session, which application::stop() tears down itself in the
   * right order) must be destroyed before stop() - stop() nulls out
   * application::socketengine, which this object's destructor still needs
   * to deregister its socket. */
  client.reset();
  app.stop();
}

TEST_F(RealApiTest, CreateSessionTracksSessionAndOpensRealEventSubConnection) {
  tpp::application app(creds.client_id, next_test_port(), creds.client_secret);
  app.start(false);

  /* This is not a real user token (obtaining one needs an interactive
   * browser round trip through the implicit grant flow), so the automatic
   * channel.chat.message subscription this triggers is expected to fail -
   * that's fine, this test is only about application/session
   * bookkeeping and the EventSub connection itself, both of which are real. */
  auto session =
      app.create_session("000000000", "faketestuser", "invalid_fake_token");
  ASSERT_NE(session, nullptr);
  EXPECT_EQ(session->get_user_id(), "000000000");
  EXPECT_EQ(session->get_login(), "faketestuser");
  EXPECT_EQ(app.get_session("000000000"), session);
  EXPECT_EQ(app.get_sessions().size(), 1u);

  std::this_thread::sleep_for(std::chrono::seconds(2));

  app.remove_session("000000000");
  EXPECT_EQ(app.get_session("000000000"), nullptr);
  EXPECT_TRUE(app.get_sessions().empty());

  /* remove_session() only drops the application's own reference; this
   * local shared_ptr is a separate one and must go too, for the same
   * reason client.reset() precedes stop() in the EventSub test above. */
  session.reset();
  app.stop();
}
