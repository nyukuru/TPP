#include <gtest/gtest.h>
#include <tpp/application.h>
#include <tpp/event.h>
#include <tpp/session.h>

#include <atomic>
#include <memory>
#include <string>

namespace {
uint16_t next_test_port() {
  static std::atomic<uint16_t> port {23000};
  return port.fetch_add(1);
}
}// namespace

TEST(Event, FindHandlerReturnsHandlerForChatMessage) {
  EXPECT_NE(tpp::events::find_handler("channel.chat.message"), nullptr);
}

TEST(Event, FindHandlerReturnsHandlerForEveryDocumentedSubscriptionType) {
  static constexpr const char *some_other_types[] = {
      "channel.follow", "channel.subscribe", "channel.cheer",
      "channel.raid",   "channel.ban",       "stream.online",
      "stream.offline", "user.update",       "user.whisper.message",
  };
  for (const char *type : some_other_types) {
    EXPECT_NE(tpp::events::find_handler(type), nullptr) << type;
  }
}

TEST(Event, FindHandlerReturnsNullForUnknownSubscriptionType) {
  EXPECT_EQ(tpp::events::find_handler("not.a.real.type"), nullptr);
}

TEST(Event, ChatMessageHandlerDispatchesToOnChatMessageHook) {
  tpp::application app {"test_client_id", next_test_port()};
  auto             session =
      std::make_shared<tpp::session>(&app, "1971641", "streamer", "tok_abc");

  bool called = false;
  session->on_chat_message([&](const tpp::chat_message_t &event) {
    called = true;
    EXPECT_EQ(event.from, session.get());
    EXPECT_EQ(event.broadcaster.id, "1971641");
    EXPECT_EQ(event.broadcaster.login, "streamer");
    EXPECT_EQ(event.chatter.id, "4145994");
    EXPECT_EQ(event.chatter.login, "viewer32");
    EXPECT_EQ(event.message, "ping");
    EXPECT_FALSE(event.raw_event.empty());
  });

  const tpp::events::event_handler *handler =
      tpp::events::find_handler("channel.chat.message");
  ASSERT_NE(handler, nullptr);

  std::string    event_json = R"({
    "broadcaster_user_id": "1971641",
    "broadcaster_user_login": "streamer",
    "broadcaster_user_name": "streamer",
    "chatter_user_id": "4145994",
    "chatter_user_login": "viewer32",
    "chatter_user_name": "viewer32",
    "message": {"text": "ping"}
  })";
  nlohmann::json event      = nlohmann::json::parse(event_json);
  handler->handle(session.get(), event, event_json);

  EXPECT_TRUE(called);
}

TEST(Event, NoOpHandlerDoesNotCrash) {
  tpp::application app {"test_client_id", next_test_port()};
  auto session = std::make_shared<tpp::session>(&app, "1", "u", "tok");

  const tpp::events::event_handler *handler =
      tpp::events::find_handler("channel.follow");
  ASSERT_NE(handler, nullptr);
  nlohmann::json event = nlohmann::json::parse("{}");
  EXPECT_NO_THROW({ handler->handle(session.get(), event, "{}"); });
}
