#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>
#include <thread>

#include "tpp/cluster.h"

class ClusterTest : public ::testing::Test {
 protected:
  const std::string valid_token = "oauth:test_token_12345";
  const std::string empty_token = "";
};

TEST_F(ClusterTest, ConstructorWithValidToken) {
  EXPECT_NO_THROW({ tpp::cluster bot(valid_token); });
}

TEST_F(ClusterTest, ConstructorWithEmptyToken) {
  EXPECT_THROW({ tpp::cluster bot(empty_token); }, std::invalid_argument);
}

TEST_F(ClusterTest, ConstructorWithIntents) {
  tpp::cluster bot(valid_token,
                   tpp::intents::chat_messages | tpp::intents::follows);
  EXPECT_EQ(bot.get_intents(),
            tpp::intents::chat_messages | tpp::intents::follows);
}

TEST_F(ClusterTest, GetToken) {
  tpp::cluster bot(valid_token);
  EXPECT_EQ(bot.get_token(), valid_token);
}

TEST_F(ClusterTest, GetIntents) {
  tpp::cluster bot(valid_token, tpp::intents::all);
  EXPECT_EQ(bot.get_intents(), tpp::intents::all);
}

TEST_F(ClusterTest, DefaultIntents) {
  tpp::cluster bot(valid_token);
  EXPECT_EQ(bot.get_intents(), tpp::intents::chat_messages);
}

TEST_F(ClusterTest, InitiallyNotRunning) {
  tpp::cluster bot(valid_token);
  EXPECT_FALSE(bot.is_running());
}

TEST_F(ClusterTest, StartNonBlocking) {
  tpp::cluster bot(valid_token);

  bot.start(false);
  EXPECT_TRUE(bot.is_running());

  bot.stop();
  EXPECT_FALSE(bot.is_running());
}

TEST_F(ClusterTest, DoubleStart) {
  tpp::cluster bot(valid_token);

  bot.start(false);
  EXPECT_THROW({ bot.start(false); }, std::runtime_error);

  bot.stop();
}

TEST_F(ClusterTest, StopWhenNotRunning) {
  tpp::cluster bot(valid_token);

  EXPECT_NO_THROW({ bot.stop(); });
}

TEST_F(ClusterTest, JoinChannelValidName) {
  tpp::cluster bot(valid_token);

  EXPECT_NO_THROW({ bot.join_channel("test_channel"); });
}

TEST_F(ClusterTest, JoinChannelEmptyName) {
  tpp::cluster bot(valid_token);

  EXPECT_THROW({ bot.join_channel(""); }, std::invalid_argument);
}

TEST_F(ClusterTest, LeaveChannelValidName) {
  tpp::cluster bot(valid_token);

  EXPECT_NO_THROW({ bot.leave_channel("test_channel"); });
}

TEST_F(ClusterTest, LeaveChannelEmptyName) {
  tpp::cluster bot(valid_token);

  EXPECT_THROW({ bot.leave_channel(""); }, std::invalid_argument);
}

TEST_F(ClusterTest, SendMessageValidParameters) {
  tpp::cluster bot(valid_token);

  EXPECT_NO_THROW({ bot.send_message("test_channel", "Hello, Twitch!"); });
}

TEST_F(ClusterTest, SendMessageEmptyChannel) {
  tpp::cluster bot(valid_token);

  EXPECT_THROW(
      { bot.send_message("", "Hello, Twitch!"); }, std::invalid_argument);
}

TEST_F(ClusterTest, SendMessageEmptyMessage) {
  tpp::cluster bot(valid_token);

  EXPECT_THROW(
      { bot.send_message("test_channel", ""); }, std::invalid_argument);
}

TEST_F(ClusterTest, IntentsBitwiseOperations) {
  auto combined = tpp::intents::chat_messages | tpp::intents::follows;
  EXPECT_TRUE((combined & tpp::intents::chat_messages) ==
              tpp::intents::chat_messages);
  EXPECT_TRUE((combined & tpp::intents::follows) == tpp::intents::follows);
  EXPECT_TRUE((combined & tpp::intents::raids) == tpp::intents::none);
}

TEST_F(ClusterTest, DestructorStopsRunningBot) {
  {
    tpp::cluster bot(valid_token);
    bot.start(false);
    EXPECT_TRUE(bot.is_running());
  }
  // Bot should be stopped after destructor
}