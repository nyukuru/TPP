#include <gtest/gtest.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <thread>

class ConduitTest : public ::testing::Test {
 protected:
  const std::string client_id = "test_client_id";
};

TEST_F(ConduitTest, ConstructorWithValidArgs) {
  EXPECT_NO_THROW({ tpp::conduit app(client_id); });
}

TEST_F(ConduitTest, ConstructorEmptyClientIdThrows) {
  EXPECT_THROW({ tpp::conduit app(""); }, std::invalid_argument);
}

TEST_F(ConduitTest, GetClientIdMatchesConstructorArgument) {
  tpp::conduit app(client_id);
  EXPECT_EQ(app.get_client_id(), client_id);
}

TEST_F(ConduitTest, DefaultIntentsIsChatMessages) {
  tpp::conduit app(client_id);
  EXPECT_TRUE(app.get_intents().has(tpp::i_chat_messages));
  EXPECT_FALSE(app.get_intents().has(tpp::i_follows));
}

TEST_F(ConduitTest, CustomIntents) {
  tpp::conduit app(client_id, "", tpp::intent(tpp::i_chat_messages, tpp::i_follows));
  EXPECT_TRUE(app.get_intents().has(tpp::i_chat_messages, tpp::i_follows));
  EXPECT_FALSE(app.get_intents().has(tpp::i_channel_points));
}

TEST_F(ConduitTest, IntentsBitwiseOperations) {
  auto combined = tpp::i_chat_messages | tpp::i_follows;
  EXPECT_TRUE((combined & tpp::i_chat_messages).has(tpp::i_chat_messages));
  EXPECT_TRUE((combined & tpp::i_follows).has(tpp::i_follows));
  EXPECT_FALSE((combined & tpp::i_raids).has_any(tpp::i_chat_messages, tpp::i_channel_points, tpp::i_subscriptions, tpp::i_follows, tpp::i_raids));
}

TEST_F(ConduitTest, InitiallyNotRunning) {
  tpp::conduit app(client_id);
  EXPECT_FALSE(app.is_running());
}

TEST_F(ConduitTest, StartNonBlockingThenStop) {
  tpp::conduit app(client_id);

  app.start(false);
  EXPECT_TRUE(app.is_running());

  app.stop();
  EXPECT_FALSE(app.is_running());
}

TEST_F(ConduitTest, DoubleStartThrows) {
  tpp::conduit app(client_id);

  app.start(false);
  EXPECT_THROW({ app.start(false); }, std::runtime_error);

  app.stop();
}

TEST_F(ConduitTest, StopWhenNotRunningIsNoOp) {
  tpp::conduit app(client_id);
  EXPECT_NO_THROW({ app.stop(); });
}

TEST_F(ConduitTest, DestructorStopsRunningConduit) {
  {
    tpp::conduit app(client_id);
    app.start(false);
    EXPECT_TRUE(app.is_running());
  }
  /* No crash/hang on scope exit is the assertion here. */
}

TEST_F(ConduitTest, CreateConsumerBeforeStartSucceeds) {
  tpp::conduit app(client_id);
  auto consumer = app.create_consumer("12345", "someuser", "faketoken");
  ASSERT_NE(consumer, nullptr);
  EXPECT_EQ(consumer->get_user_id(), "12345");
}

TEST_F(ConduitTest, CreateConsumerDoesNotTrackIt) {
  tpp::conduit app(client_id);
  app.create_consumer("12345", "someuser", "faketoken");
  EXPECT_EQ(app.get_consumer("12345"), nullptr);
  EXPECT_TRUE(app.get_consumers().empty());
}

TEST_F(ConduitTest, OpenConduitBeforeStartThrows) {
  tpp::conduit app(client_id);
  EXPECT_THROW({ app.open_conduit(1); }, std::runtime_error);
}

TEST_F(ConduitTest, OpenConduitWithZeroShardsThrows) {
  tpp::conduit app(client_id);
  app.start(false);
  EXPECT_THROW({ app.open_conduit(0); }, std::invalid_argument);
}

TEST_F(ConduitTest, GetShardCountInitiallyZero) {
  tpp::conduit app(client_id);
  EXPECT_EQ(app.get_shard_count(), 0u);
}

TEST_F(ConduitTest, ConstructorWithShardCountDoesNotAutoOpenBeforeStart) {
  tpp::conduit app(client_id, "", tpp::intent(tpp::i_chat_messages), 2);
  EXPECT_EQ(app.get_shard_count(), 0u);
}

TEST_F(ConduitTest, ConstructorWithShardCountButNoSecretDoesNotCrashOnStart) {
  /* open_conduit() requires client_secret to obtain an app access token;
   * without one, start()'s attempt to honor the requested shard count
   * just logs and gives up rather than throwing or hanging. */
  tpp::conduit app(client_id, "", tpp::intent(tpp::i_chat_messages), 2);
  EXPECT_NO_THROW({ app.start(false); });
  EXPECT_EQ(app.get_shard_count(), 0u);
  app.stop();
}

TEST_F(ConduitTest, AddConsumerMakesItFindable) {
  tpp::conduit app(client_id);
  auto consumer = app.create_consumer("12345", "someuser", "faketoken");
  app.add_consumer(consumer);
  EXPECT_EQ(app.get_consumer("12345"), consumer);
  EXPECT_EQ(app.get_consumers().size(), 1u);
}

TEST_F(ConduitTest, AddConsumerReplacesExistingEntryForSameUser) {
  tpp::conduit app(client_id);
  auto first = app.create_consumer("12345", "someuser", "tok1");
  auto second = app.create_consumer("12345", "someuser", "tok2");
  app.add_consumer(first);
  app.add_consumer(second);
  EXPECT_EQ(app.get_consumer("12345"), second);
  EXPECT_EQ(app.get_consumers().size(), 1u);
}

TEST_F(ConduitTest, GetConsumerOnUnknownUserReturnsNull) {
  tpp::conduit app(client_id);
  EXPECT_EQ(app.get_consumer("nonexistent"), nullptr);
}

TEST_F(ConduitTest, GetConsumersInitiallyEmpty) {
  tpp::conduit app(client_id);
  EXPECT_TRUE(app.get_consumers().empty());
}

TEST_F(ConduitTest, RemoveConsumerOnUnknownUserIsNoOp) {
  tpp::conduit app(client_id);
  EXPECT_NO_THROW({ app.remove_consumer("nonexistent"); });
}

/* conduit::open_conduit() opens real EventSub WebSocket connections, so
 * exercising it end to end (populating get_consumer()/get_consumers(),
 * removing it again, etc.) belongs in the functional suite, which is
 * allowed to touch the real network. See tests/functional/test_real_api.cpp.
 */

TEST_F(ConduitTest, GetAppAccessTokenWithoutSecretFailsSynchronously) {
  tpp::conduit app(client_id);

  bool called = false;
  bool success = true;
  app.get_app_access_token([&](bool ok, const std::string &, uint64_t) {
    called = true;
    success = ok;
  });

  EXPECT_TRUE(called);
  EXPECT_FALSE(success);
}

TEST_F(ConduitTest, RefreshUserTokenWithoutSecretFailsSynchronously) {
  tpp::conduit app(client_id);

  bool called = false;
  bool success = true;
  app.refresh_user_token("some_refresh_token", [&](bool ok, const std::string &, const std::string &, uint64_t) {
    called = true;
    success = ok;
  });

  EXPECT_TRUE(called);
  EXPECT_FALSE(success);
}

TEST_F(ConduitTest, TimerStartAndStopDoNotCrash) {
  tpp::conduit app(client_id);

  bool fired = false;
  tpp::timer handle = app.start_timer([&](tpp::timer) { fired = true; }, 3600);
  EXPECT_NE(handle, 0u);
  EXPECT_TRUE(app.stop_timer(handle));
  (void) fired;
}

TEST_F(ConduitTest, TickTimersFiresDueTimer) {
  tpp::conduit app(client_id);

  bool fired = false;
  app.start_timer([&](tpp::timer) { fired = true; }, 0);

  /* start_timer schedules next_tick at time(nullptr) + frequency; with
   * frequency 0 it is already due, so a single tick_timers() call fires
   * it. */
  app.tick_timers();
  EXPECT_TRUE(fired);
}
