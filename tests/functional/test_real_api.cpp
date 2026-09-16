#include <gtest/gtest.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>
#include <tpp/https_client.h>

#include <atomic>
#include <cctype>
#include <chrono>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

namespace {

struct test_credentials {
  std::string client_id;
  std::string client_secret;
  bool loaded {false};
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
    std::string key = trim(line.substr(0, eq));
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
bool wait_for(Pred pred, std::chrono::milliseconds timeout = std::chrono::seconds(15)) {
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
  uint16_t status {0};
  std::string body;
};

http_result do_request(tpp::conduit &app, const std::string &host, const std::string &path, const std::string &verb, const std::string &body,
                       const tpp::http_headers &headers) {
  http_result result;
  std::atomic<bool> done {false};

  auto client = std::make_unique<tpp::https_client>(&app, host, 443, path, verb, body, headers, false, 10, "1.1", [&](tpp::https_client *c) {
    result.status = c->get_status();
    result.body = c->get_content();
    done = true;
  });

  wait_for([&] { return done.load(); });
  return result;
}

/* Fetches a real app access token via the Client Credentials Grant,
 * skipping the calling test if that fails - the token tests below build on
 * top of this. */
bool fetch_app_token(tpp::conduit &app, std::string &token, uint64_t &expires_in) {
  std::atomic<bool> done {false};
  bool ok = false;
  app.get_app_access_token([&](bool success, const std::string &tok, uint64_t exp) {
    ok = success;
    token = tok;
    expires_in = exp;
    done = true;
  });
  wait_for([&] { return done.load(); });
  return ok;
}

/* Twitch does not clean up conduits on its own - a client_id is capped at
 * 5 of them - so every test that opens one must delete it again, or a few
 * runs of this suite permanently locks the test client_id out of creating
 * new ones. Must be called before stop(), which does not itself delete
 * the conduit (a real bot is meant to keep reusing the same one). */
void delete_conduit_and_wait(tpp::conduit &app) {
  std::atomic<bool> done {false};
  app.delete_conduit([&](bool) { done = true; });
  EXPECT_TRUE(wait_for([&] { return done.load(); }));
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
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));
  EXPECT_FALSE(token.empty());
  EXPECT_GT(expires_in, 0u);

  app.stop();
}

TEST_F(RealApiTest, ClientCredentialsGrantWithBadSecretFails) {
  tpp::conduit app(creds.client_id, "definitely_not_the_secret");
  app.start(false);

  std::string token;
  uint64_t expires_in = 0;
  EXPECT_FALSE(fetch_app_token(app, token, expires_in));
  EXPECT_TRUE(token.empty());

  app.stop();
}

TEST_F(RealApiTest, RefreshUserTokenWithFakeTokenFails) {
  /* We don't have a real user refresh token to test rotation with (getting
   * one needs an interactive browser round trip through a grant type
   * tpp::conduit doesn't implement yet), but this still exercises the
   * real request path - grant_type=refresh_token against the real token
   * endpoint - and confirms Twitch's rejection of a bogus refresh token is
   * correctly reported as failure. */
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::atomic<bool> done {false};
  bool ok = true;
  app.refresh_user_token("not_a_real_refresh_token", [&](bool success, const std::string &, const std::string &, uint64_t) {
    ok = success;
    done = true;
  });

  ASSERT_TRUE(wait_for([&] { return done.load(); }));
  EXPECT_FALSE(ok);

  app.stop();
}

TEST_F(RealApiTest, AppAccessTokenValidates) {
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));

  /* Twitch's /oauth2/validate endpoint specifically wants the "OAuth"
   * auth scheme rather than "Bearer". */
  tpp::http_headers headers {
      {"Authorization", "OAuth " + token},
  };
  http_result result = do_request(app, "id.twitch.tv", "/oauth2/validate", "GET", "", headers);

  EXPECT_EQ(result.status, 200);
  EXPECT_NE(result.body.find(creds.client_id), std::string::npos);

  app.stop();
}

TEST_F(RealApiTest, HelixGamesLookupWithAppToken) {
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::string token;
  uint64_t expires_in = 0;
  ASSERT_TRUE(fetch_app_token(app, token, expires_in));

  tpp::http_headers headers {
      {"Authorization", "Bearer " + token},
      {"Client-Id", creds.client_id},
  };
  http_result result = do_request(app, "api.twitch.tv", "/helix/games?name=Just%20Chatting", "GET", "", headers);

  EXPECT_EQ(result.status, 200);
  EXPECT_NE(result.body.find("\"data\""), std::string::npos);

  app.stop();
}

TEST_F(RealApiTest, EventSubWebSocketConnectsAndWelcomes) {
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::mutex mutex;
  std::atomic<bool> welcomed {false};
  std::string session_id;

  auto client = std::make_unique<tpp::eventsub_client>(&app);
  client->on_welcome([&](const tpp::eventsub_welcome_t &w) {
    std::lock_guard<std::mutex> lock(mutex);
    session_id = w.session_id;
    welcomed = true;
  });

  ASSERT_TRUE(wait_for([&] { return welcomed.load(); }));
  {
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_FALSE(session_id.empty());
  }

  /* Connections the caller constructs directly (rather than ones owned by
   * a conduit's shard pool, which conduit::stop() tears down itself in the
   * right order) must be destroyed before stop() - stop() nulls out
   * conduit::socketengine, which this object's destructor still needs
   * to deregister its socket. */
  client.reset();
  app.stop();
}

TEST_F(RealApiTest, OpenConduitCreatesRealConduitAndWelcomesShards) {
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::atomic<bool> ready {false};
  bool ok = false;
  app.open_conduit(1, [&](bool success) {
    ok = success;
    ready = true;
  });

  ASSERT_TRUE(wait_for([&] { return ready.load(); }));
  EXPECT_TRUE(ok);
  EXPECT_FALSE(app.get_conduit_id().empty());
  EXPECT_EQ(app.get_shard_count(), 1u);

  delete_conduit_and_wait(app);
  EXPECT_EQ(app.get_shard_count(), 0u);
  app.stop();
}

TEST_F(RealApiTest, ShardCountAtConstructionAutoOpensConduitOnStart) {
  tpp::conduit app(creds.client_id, creds.client_secret, tpp::intent(tpp::i_chat_messages), 1);
  EXPECT_EQ(app.get_shard_count(), 0u);

  app.start(false);

  ASSERT_TRUE(wait_for([&] { return app.get_shard_count() == 1u; }));
  EXPECT_FALSE(app.get_conduit_id().empty());

  delete_conduit_and_wait(app);
  app.stop();
}

TEST_F(RealApiTest, CreateConsumerTracksItAndSubscribesThroughConduit) {
  tpp::conduit app(creds.client_id, creds.client_secret);
  app.start(false);

  std::atomic<bool> conduit_ready {false};
  app.open_conduit(1, [&](bool) { conduit_ready = true; });
  ASSERT_TRUE(wait_for([&] { return conduit_ready.load(); }));

  auto consumer = app.create_consumer("000000000", "faketestuser", "invalid_fake_token");
  ASSERT_NE(consumer, nullptr);
  EXPECT_EQ(consumer->get_user_id(), "000000000");
  EXPECT_EQ(consumer->get_login(), "faketestuser");

  /* create_consumer() does not track the consumer by itself - that's an
   * explicit opt-in. */
  EXPECT_EQ(app.get_consumer("000000000"), nullptr);
  app.add_consumer(consumer);
  EXPECT_EQ(app.get_consumer("000000000"), consumer);
  EXPECT_EQ(app.get_consumers().size(), 1u);

  /* This is not a real user token (obtaining one needs an interactive
   * browser round trip through the implicit grant flow), so this
   * subscription attempt is expected to fail - that's fine, this test is
   * only about conduit/consumer bookkeeping and the conduit subscription
   * request path, both of which are real. */
  consumer->subscribe(tpp::event::channel_chat_message(consumer->get_user_id()));

  std::this_thread::sleep_for(std::chrono::seconds(2));

  app.remove_consumer("000000000");
  EXPECT_EQ(app.get_consumer("000000000"), nullptr);
  EXPECT_TRUE(app.get_consumers().empty());

  /* remove_consumer() only drops the conduit's own reference; this local
   * shared_ptr is a separate one and must go too, for the same reason
   * client.reset() precedes stop() in the EventSub test above. */
  consumer.reset();
  delete_conduit_and_wait(app);
  app.stop();
}
