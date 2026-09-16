#include <gtest/gtest.h>
#include <tpp/application.h>
#include <tpp/scope.h>
#include <tpp/session.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <thread>

namespace {
/* Ports are picked per-test to avoid TIME_WAIT collisions between tests
 * that bind and quickly tear down a local listener on the same port. */
uint16_t next_test_port() {
  static std::atomic<uint16_t> port {21000};
  return port.fetch_add(1);
}
}// namespace

class ApplicationTest : public ::testing::Test {
 protected:
  const std::string client_id = "test_client_id";
};

TEST_F(ApplicationTest, ConstructorWithValidArgs) {
  EXPECT_NO_THROW({ tpp::application app(client_id, next_test_port()); });
}

TEST_F(ApplicationTest, ConstructorEmptyClientIdThrows) {
  EXPECT_THROW(
      { tpp::application app("", next_test_port()); }, std::invalid_argument);
}

TEST_F(ApplicationTest, ConstructorZeroPortThrows) {
  EXPECT_THROW({ tpp::application app(client_id, 0); }, std::invalid_argument);
}

TEST_F(ApplicationTest, GetClientIdMatchesConstructorArgument) {
  tpp::application app(client_id, next_test_port());
  EXPECT_EQ(app.get_client_id(), client_id);
}

TEST_F(ApplicationTest, DefaultIntentsIsChatMessages) {
  tpp::application app(client_id, next_test_port());
  EXPECT_TRUE(app.get_intents().has(tpp::i_chat_messages));
  EXPECT_FALSE(app.get_intents().has(tpp::i_follows));
}

TEST_F(ApplicationTest, CustomIntents) {
  tpp::application app(client_id, next_test_port(), "",
                       tpp::intent(tpp::i_chat_messages, tpp::i_follows));
  EXPECT_TRUE(app.get_intents().has(tpp::i_chat_messages, tpp::i_follows));
  EXPECT_FALSE(app.get_intents().has(tpp::i_channel_points));
}

TEST_F(ApplicationTest, IntentsBitwiseOperations) {
  auto combined = tpp::i_chat_messages | tpp::i_follows;
  EXPECT_TRUE((combined & tpp::i_chat_messages).has(tpp::i_chat_messages));
  EXPECT_TRUE((combined & tpp::i_follows).has(tpp::i_follows));
  EXPECT_FALSE((combined & tpp::i_raids)
                   .has_any(tpp::i_chat_messages, tpp::i_channel_points,
                            tpp::i_subscriptions, tpp::i_follows,
                            tpp::i_raids));
}

TEST_F(ApplicationTest, InitiallyNotRunning) {
  tpp::application app(client_id, next_test_port());
  EXPECT_FALSE(app.is_running());
}

TEST_F(ApplicationTest, StartNonBlockingThenStop) {
  tpp::application app(client_id, next_test_port());

  app.start(false);
  EXPECT_TRUE(app.is_running());

  app.stop();
  EXPECT_FALSE(app.is_running());
}

TEST_F(ApplicationTest, DoubleStartThrows) {
  tpp::application app(client_id, next_test_port());

  app.start(false);
  EXPECT_THROW({ app.start(false); }, std::runtime_error);

  app.stop();
}

TEST_F(ApplicationTest, StopWhenNotRunningIsNoOp) {
  tpp::application app(client_id, next_test_port());
  EXPECT_NO_THROW({ app.stop(); });
}

TEST_F(ApplicationTest, DestructorStopsRunningApplication) {
  {
    tpp::application app(client_id, next_test_port());
    app.start(false);
    EXPECT_TRUE(app.is_running());
  }
  /* No crash/hang on scope exit is the assertion here. */
}

TEST_F(ApplicationTest, CreateSessionBeforeStartThrows) {
  tpp::application app(client_id, next_test_port());
  EXPECT_THROW(
      { app.create_session("12345", "someuser", "faketoken"); },
      std::runtime_error);
}

TEST_F(ApplicationTest, GetSessionOnUnknownUserReturnsNull) {
  tpp::application app(client_id, next_test_port());
  EXPECT_EQ(app.get_session("nonexistent"), nullptr);
}

TEST_F(ApplicationTest, GetSessionsInitiallyEmpty) {
  tpp::application app(client_id, next_test_port());
  EXPECT_TRUE(app.get_sessions().empty());
}

TEST_F(ApplicationTest, RemoveSessionOnUnknownUserIsNoOp) {
  tpp::application app(client_id, next_test_port());
  EXPECT_NO_THROW({ app.remove_session("nonexistent"); });
}

/* application::create_session() opens a real EventSub WebSocket connection
 * as part of session->connect(), so exercising it end to end (populating
 * get_session()/get_sessions(), removing it again, etc.) belongs in the
 * functional suite, which is allowed to touch the real network. See
 * tests/functional/test_real_api.cpp. */

TEST_F(ApplicationTest, GenerateAuthUrlContainsExpectedComponents) {
  tpp::application app(client_id, next_test_port());
  tpp::scope       scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);

  std::string url = app.generate_auth_url(scopes);

  EXPECT_NE(url.find("https://id.twitch.tv/oauth2/authorize"),
            std::string::npos);
  EXPECT_NE(url.find("response_type=token"), std::string::npos);
  EXPECT_EQ(url.find("id_token"), std::string::npos)
      << "response_type should not request an id_token without the openid "
         "scope";
  EXPECT_NE(url.find("client_id=" + client_id), std::string::npos);
  EXPECT_NE(url.find("chat%3Aread"), std::string::npos);
  EXPECT_NE(url.find("chat%3Aedit"), std::string::npos);
  EXPECT_NE(url.find("&state="), std::string::npos);
  EXPECT_EQ(url.find("&nonce="), std::string::npos);
  EXPECT_EQ(url.find("&claims="), std::string::npos);
}

TEST_F(ApplicationTest, GenerateAuthUrlWithOpenidRequestsIdToken) {
  tpp::application app(client_id, next_test_port());
  tpp::scope       scopes(tpp::scope::s_chat_read, tpp::scope::s_openid);

  std::string url = app.generate_auth_url(scopes);

  EXPECT_NE(url.find("response_type=token+id_token"), std::string::npos);
  EXPECT_NE(url.find("&nonce="), std::string::npos);
}

TEST_F(ApplicationTest, GenerateAuthUrlIncludesClaimsWhenGiven) {
  tpp::application app(client_id, next_test_port());
  tpp::scope       scopes(tpp::scope::s_chat_read);

  std::string url =
      app.generate_auth_url(scopes, R"({"userinfo":{"email":null}})");

  EXPECT_NE(url.find("&claims="), std::string::npos);
}

TEST_F(ApplicationTest, GenerateAuthUrlStateChangesEachCall) {
  tpp::application app(client_id, next_test_port());
  tpp::scope       scopes(tpp::scope::s_chat_read);

  std::string url1 = app.generate_auth_url(scopes);
  std::string url2 = app.generate_auth_url(scopes);

  auto extract_state = [](const std::string &url) {
    auto pos = url.find("&state=");
    return url.substr(pos);
  };
  EXPECT_NE(extract_state(url1), extract_state(url2));
}

TEST_F(ApplicationTest, GetAppAccessTokenWithoutSecretFailsSynchronously) {
  tpp::application app(client_id, next_test_port());

  bool called  = false;
  bool success = true;
  app.get_app_access_token([&](bool ok, const std::string &, uint64_t) {
    called  = true;
    success = ok;
  });

  EXPECT_TRUE(called);
  EXPECT_FALSE(success);
}

TEST_F(ApplicationTest, RefreshUserTokenWithoutSecretFailsSynchronously) {
  tpp::application app(client_id, next_test_port());

  bool called  = false;
  bool success = true;
  app.refresh_user_token(
      "some_refresh_token",
      [&](bool ok, const std::string &, const std::string &, uint64_t) {
        called  = true;
        success = ok;
      });

  EXPECT_TRUE(called);
  EXPECT_FALSE(success);
}

TEST_F(ApplicationTest, TimerStartAndStopDoNotCrash) {
  tpp::application app(client_id, next_test_port());

  bool       fired  = false;
  tpp::timer handle = app.start_timer([&](tpp::timer) { fired = true; }, 3600);
  EXPECT_NE(handle, 0u);
  EXPECT_TRUE(app.stop_timer(handle));
  (void) fired;
}

TEST_F(ApplicationTest, TickTimersFiresDueTimer) {
  tpp::application app(client_id, next_test_port());

  bool fired = false;
  app.start_timer([&](tpp::timer) { fired = true; }, 0);

  /* start_timer schedules next_tick at time(nullptr) + frequency; with
   * frequency 0 it is already due, so a single tick_timers() call fires
   * it. */
  app.tick_timers();
  EXPECT_TRUE(fired);
}
