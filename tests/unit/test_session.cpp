#include <gtest/gtest.h>
#include <tpp/application.h>
#include <tpp/event.h>
#include <tpp/eventsub.h>
#include <tpp/session.h>
#include <tpp/user.h>

#include <atomic>
#include <ctime>
#include <memory>
#include <string>

namespace {
uint16_t next_test_port() {
  static std::atomic<uint16_t> port {22000};
  return port.fetch_add(1);
}

/* session::connect() opens a real EventSub WebSocket connection (and, if a
 * refresh token is set, schedules rotation via it), so these tests
 * construct sessions directly and never call connect(), exercising only
 * the session's bookkeeping and hook-dispatch logic without touching the
 * network. Full connect()/subscribe/send_message/rotation coverage against
 * the real API lives in the functional suite. */
std::shared_ptr<tpp::session> make_test_session(
    tpp::application &app, const std::string &user_id = "111222333",
    const std::string &login = "testuser", const std::string &token = "tok_abc",
    const std::string &id_token = "", uint64_t expires_in = 0,
    const std::string &refresh_token = "") {
  return std::make_shared<tpp::session>(&app, user_id, login, token, id_token,
                                        expires_in, refresh_token);
}
}// namespace

class SessionTest : public ::testing::Test {
 protected:
  tpp::application app {"test_client_id", next_test_port()};
};

TEST_F(SessionTest, GettersReturnConstructorArguments) {
  auto session =
      make_test_session(app, "42", "someuser", "sometoken", "idtok", 3600);

  EXPECT_EQ(session->get_user_id(), "42");
  EXPECT_EQ(session->get_login(), "someuser");
  EXPECT_EQ(session->get_access_token(), "sometoken");
  EXPECT_EQ(session->get_id_token(), "idtok");
  EXPECT_EQ(session->get_token_expires_in(), 3600u);
}

TEST_F(SessionTest, GetClientIdForwardsToOwningApplication) {
  auto session = make_test_session(app);
  EXPECT_EQ(session->get_client_id(), app.get_client_id());
}

TEST_F(SessionTest, GetApplicationReturnsOwner) {
  auto session = make_test_session(app);
  EXPECT_EQ(&session->get_application(), &app);
}

TEST_F(SessionTest, DispatchChatMessageWithNoHookDoesNotCrash) {
  auto                session = make_test_session(app);
  tpp::chat_message_t event;
  event.broadcaster = {"123", "somechannel", "SomeChannel"};
  event.chatter     = {"456", "someuser", "SomeUser"};
  event.message     = "hello";
  EXPECT_NO_THROW({ session->on_chat_message.call(event); });
}

TEST_F(SessionTest, OnChatMessageHookFiresOnDispatch) {
  auto session = make_test_session(app);

  bool                called = false;
  tpp::chat_message_t seen;
  session->on_chat_message([&](const tpp::chat_message_t &event) {
    called = true;
    seen   = event;
  });

  tpp::chat_message_t event;
  event.broadcaster = {"1971641", "streamer", "Streamer"};
  event.chatter     = {"4145994", "viewer32", "Viewer32"};
  event.message     = "ping";
  session->on_chat_message.call(event);

  EXPECT_TRUE(called);
  EXPECT_EQ(seen.broadcaster, event.broadcaster);
  EXPECT_EQ(seen.chatter, event.chatter);
  EXPECT_EQ(seen.message, "ping");
}

TEST_F(SessionTest, OnChatMessageSupportsMultipleHooks) {
  auto session = make_test_session(app);

  int calls_to_first  = 0;
  int calls_to_second = 0;

  session->on_chat_message(
      [&](const tpp::chat_message_t &) { ++calls_to_first; });
  session->on_chat_message(
      [&](const tpp::chat_message_t &) { ++calls_to_second; });

  session->on_chat_message.call(tpp::chat_message_t {});

  EXPECT_EQ(calls_to_first, 1);
  EXPECT_EQ(calls_to_second, 1);
}

TEST_F(SessionTest, OnChatMessageDetachStopsFiring) {
  auto session = make_test_session(app);

  int               calls = 0;
  tpp::event_handle handle =
      session->on_chat_message([&](const tpp::chat_message_t &) { ++calls; });
  session->on_chat_message.detach(handle);

  session->on_chat_message.call(tpp::chat_message_t {});

  EXPECT_EQ(calls, 0);
}

TEST_F(SessionTest, DispatchCalledMultipleTimesAccumulates) {
  auto session = make_test_session(app);

  int count = 0;
  session->on_chat_message([&](const tpp::chat_message_t &) { ++count; });

  tpp::chat_message_t event;
  session->on_chat_message.call(event);
  session->on_chat_message.call(event);
  session->on_chat_message.call(event);

  EXPECT_EQ(count, 3);
}

TEST_F(SessionTest, DefaultIdTokenAndExpiryAreEmptyAndZero) {
  auto session = make_test_session(app, "1", "u", "tok");
  EXPECT_EQ(session->get_id_token(), "");
  EXPECT_EQ(session->get_token_expires_in(), 0u);
}

TEST_F(SessionTest, HasRefreshTokenFalseByDefault) {
  auto session = make_test_session(app);
  EXPECT_FALSE(session->has_refresh_token());
  EXPECT_EQ(session->get_refresh_token(), "");
}

TEST_F(SessionTest, HasRefreshTokenTrueWhenProvided) {
  auto session =
      make_test_session(app, "1", "u", "tok", "", 3600, "my_refresh_token");
  EXPECT_TRUE(session->has_refresh_token());
  EXPECT_EQ(session->get_refresh_token(), "my_refresh_token");
}

TEST_F(SessionTest, TokenExpiresAtIsZeroWithoutExpiresIn) {
  auto session = make_test_session(app);
  EXPECT_EQ(session->get_token_expires_at(), 0);
}

TEST_F(SessionTest, TokenExpiresAtIsComputedFromExpiresIn) {
  time_t before  = time(nullptr);
  auto   session = make_test_session(app, "1", "u", "tok", "", 3600);
  time_t after   = time(nullptr);

  /* token_expires_at_ is computed as time(nullptr) + expires_in inside the
   * constructor; allow a couple of seconds of slack either side for however
   * long the test itself took to run. */
  EXPECT_GE(session->get_token_expires_at(), before + 3600);
  EXPECT_LE(session->get_token_expires_at(), after + 3600 + 2);
}

TEST_F(SessionTest, TokenExpiresInReflectsRemainingTime) {
  auto     session   = make_test_session(app, "1", "u", "tok", "", 3600);
  uint64_t remaining = session->get_token_expires_in();
  EXPECT_GT(remaining, 0u);
  EXPECT_LE(remaining, 3600u);
}
