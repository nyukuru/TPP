#include <gtest/gtest.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>

#include <memory>
#include <string>

TEST(Event, HandleEventDispatchesToOnChatMessageHook) {
  tpp::conduit app {"test_client_id"};
  auto consumer = std::make_shared<tpp::consumer>(&app, "1971641", "streamer", "tok_abc");

  bool called = false;
  app.on_chat_message([&](const tpp::chat_message_t &event) {
    called = true;
    EXPECT_EQ(event.from, consumer.get());
    EXPECT_EQ(event.broadcaster.id, "1971641");
    EXPECT_EQ(event.chatter.id, "4145994");
    EXPECT_EQ(event.message, "ping");
  });

  std::string event_json = R"({
    "broadcaster_user_id": "1971641",
    "broadcaster_user_login": "streamer",
    "broadcaster_user_name": "streamer",
    "chatter_user_id": "4145994",
    "chatter_user_login": "viewer32",
    "chatter_user_name": "viewer32",
    "message": {"text": "ping"}
  })";
  nlohmann::json event = nlohmann::json::parse(event_json);
  tpp::events::handle_event(consumer.get(), "channel.chat.message", event, event_json);

  EXPECT_TRUE(called);
}

TEST(Event, HandleEventOnUnknownSubscriptionTypeDoesNotCrash) {
  tpp::conduit app {"test_client_id"};
  auto consumer = std::make_shared<tpp::consumer>(&app, "1", "u", "tok");

  nlohmann::json event = nlohmann::json::parse("{}");
  EXPECT_NO_THROW({ tpp::events::handle_event(consumer.get(), "not.a.real.type", event, "{}"); });
}

TEST(Event, ChatMessageHandlerWithMissingFieldsYieldsEmptyFields) {
  tpp::conduit app {"test_client_id"};
  auto consumer = std::make_shared<tpp::consumer>(&app, "1", "u", "tok");

  bool called = false;
  app.on_chat_message([&](const tpp::chat_message_t &event) {
    called = true;
    EXPECT_EQ(event.broadcaster.id, "");
    EXPECT_EQ(event.message, "");
  });

  const tpp::events::event_handler *handler = tpp::events::find_handler("channel.chat.message");
  ASSERT_NE(handler, nullptr);

  nlohmann::json event = nlohmann::json::parse(R"({"message":{}})");
  EXPECT_NO_THROW({ handler->handle(consumer.get(), event, "{}"); });
  EXPECT_TRUE(called);
}
