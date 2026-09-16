#include <gtest/gtest.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/user.h>

#include <ctime>
#include <memory>
#include <string>

namespace {
/* consumer::subscribe() routes through the owning conduit's EventSub
 * conduit, which opens real WebSocket connections, so these tests construct
 * consumers directly and never call subscribe(), exercising only the
 * consumer's bookkeeping without touching the network. Full
 * subscribe()/send_message/rotation coverage against the real API lives in
 * the functional suite. */
std::shared_ptr<tpp::consumer> make_test_consumer(tpp::conduit &app, const std::string &user_id = "111222333", const std::string &login = "testuser",
                                                  const std::string &token = "tok_abc", const std::string &id_token = "", uint64_t expires_in = 0,
                                                  const std::string &refresh_token = "") {
  return std::make_shared<tpp::consumer>(&app, user_id, login, token, id_token, expires_in, refresh_token);
}
}// namespace

class ConsumerTest : public ::testing::Test {
 protected:
  tpp::conduit app {"test_client_id"};
};

TEST_F(ConsumerTest, GettersReturnConstructorArguments) {
  auto consumer = make_test_consumer(app, "42", "someuser", "sometoken", "idtok", 3600);

  EXPECT_EQ(consumer->get_user_id(), "42");
  EXPECT_EQ(consumer->get_login(), "someuser");
  EXPECT_EQ(consumer->get_access_token(), "sometoken");
  EXPECT_EQ(consumer->get_id_token(), "idtok");
  EXPECT_EQ(consumer->get_token_expires_in(), 3600u);
}

TEST_F(ConsumerTest, GetClientIdForwardsToOwningConduit) {
  auto consumer = make_test_consumer(app);
  EXPECT_EQ(consumer->get_client_id(), app.get_client_id());
}

TEST_F(ConsumerTest, GetConduitReturnsOwner) {
  auto consumer = make_test_consumer(app);
  EXPECT_EQ(&consumer->get_conduit(), &app);
}

TEST_F(ConsumerTest, DefaultIdTokenAndExpiryAreEmptyAndZero) {
  auto consumer = make_test_consumer(app, "1", "u", "tok");
  EXPECT_EQ(consumer->get_id_token(), "");
  EXPECT_EQ(consumer->get_token_expires_in(), 0u);
}

TEST_F(ConsumerTest, HasRefreshTokenFalseByDefault) {
  auto consumer = make_test_consumer(app);
  EXPECT_FALSE(consumer->has_refresh_token());
  EXPECT_EQ(consumer->get_refresh_token(), "");
}

TEST_F(ConsumerTest, HasRefreshTokenTrueWhenProvided) {
  auto consumer = make_test_consumer(app, "1", "u", "tok", "", 3600, "my_refresh_token");
  EXPECT_TRUE(consumer->has_refresh_token());
  EXPECT_EQ(consumer->get_refresh_token(), "my_refresh_token");
}

TEST_F(ConsumerTest, TokenExpiresAtIsZeroWithoutExpiresIn) {
  auto consumer = make_test_consumer(app);
  EXPECT_EQ(consumer->get_token_expires_at(), 0);
}

TEST_F(ConsumerTest, TokenExpiresAtIsComputedFromExpiresIn) {
  time_t before = time(nullptr);
  auto consumer = make_test_consumer(app, "1", "u", "tok", "", 3600);
  time_t after = time(nullptr);

  /* token_expires_at_ is computed as time(nullptr) + expires_in inside the
   * constructor; allow a couple of seconds of slack either side for however
   * long the test itself took to run. */
  EXPECT_GE(consumer->get_token_expires_at(), before + 3600);
  EXPECT_LE(consumer->get_token_expires_at(), after + 3600 + 2);
}

TEST_F(ConsumerTest, TokenExpiresInReflectsRemainingTime) {
  auto consumer = make_test_consumer(app, "1", "u", "tok", "", 3600);
  uint64_t remaining = consumer->get_token_expires_in();
  EXPECT_GT(remaining, 0u);
  EXPECT_LE(remaining, 3600u);
}

/* on_chat_message lives on conduit (fired for every subscribed consumer,
 * filtered by chat_message_t::from), not on consumer - see
 * ConduitChatDispatchTest below. */
class ConduitChatDispatchTest : public ::testing::Test {
 protected:
  tpp::conduit app {"test_client_id"};
};

TEST_F(ConduitChatDispatchTest, DispatchChatMessageWithNoHookDoesNotCrash) {
  auto consumer = make_test_consumer(app);
  tpp::chat_message_t event;
  event.from = consumer.get();
  event.broadcaster = {"123", "somechannel", "SomeChannel"};
  event.chatter = {"456", "someuser", "SomeUser"};
  event.message = "hello";
  EXPECT_NO_THROW({ app.on_chat_message.call(event); });
}

TEST_F(ConduitChatDispatchTest, OnChatMessageHookFiresOnDispatch) {
  auto consumer = make_test_consumer(app);

  bool called = false;
  tpp::chat_message_t seen;
  app.on_chat_message([&](const tpp::chat_message_t &event) {
    called = true;
    seen = event;
  });

  tpp::chat_message_t event;
  event.from = consumer.get();
  event.broadcaster = {"1971641", "streamer", "Streamer"};
  event.chatter = {"4145994", "viewer32", "Viewer32"};
  event.message = "ping";
  app.on_chat_message.call(event);

  EXPECT_TRUE(called);
  EXPECT_EQ(seen.from, consumer.get());
  EXPECT_EQ(seen.broadcaster, event.broadcaster);
  EXPECT_EQ(seen.chatter, event.chatter);
  EXPECT_EQ(seen.message, "ping");
}

TEST_F(ConduitChatDispatchTest, OnChatMessageSupportsMultipleHooks) {
  int calls_to_first = 0;
  int calls_to_second = 0;

  app.on_chat_message([&](const tpp::chat_message_t &) { ++calls_to_first; });
  app.on_chat_message([&](const tpp::chat_message_t &) { ++calls_to_second; });

  app.on_chat_message.call(tpp::chat_message_t {});

  EXPECT_EQ(calls_to_first, 1);
  EXPECT_EQ(calls_to_second, 1);
}

TEST_F(ConduitChatDispatchTest, OnChatMessageDetachStopsFiring) {
  int calls = 0;
  tpp::event_handle handle = app.on_chat_message([&](const tpp::chat_message_t &) { ++calls; });
  app.on_chat_message.detach(handle);

  app.on_chat_message.call(tpp::chat_message_t {});

  EXPECT_EQ(calls, 0);
}

TEST_F(ConduitChatDispatchTest, DispatchCalledMultipleTimesAccumulates) {
  int count = 0;
  app.on_chat_message([&](const tpp::chat_message_t &) { ++count; });

  tpp::chat_message_t event;
  app.on_chat_message.call(event);
  app.on_chat_message.call(event);
  app.on_chat_message.call(event);

  EXPECT_EQ(count, 3);
}
