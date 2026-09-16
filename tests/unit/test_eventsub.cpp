#include <gtest/gtest.h>
#include <tpp/eventsub.h>

using tpp::chat_message_t;
using tpp::eventsub_message;
using tpp::eventsub_message_type;
using tpp::parse_chat_message_event;
using tpp::parse_eventsub_message;

/* Message shapes below follow Twitch's documented EventSub WebSocket
 * message formats:
 * https://dev.twitch.tv/docs/eventsub/handling-websocket-events/ */

TEST(EventSub, ParsesSessionWelcome) {
  std::string json = R"({
    "metadata": {
      "message_id": "96a3f3b5-5dec-4eed-908e-e11ee657416c",
      "message_type": "session_welcome",
      "message_timestamp": "2023-07-19T14:56:51.634234626Z"
    },
    "payload": {
      "session": {
        "id": "AQoQILE98gtqShGmLD7AM6yJThAB",
        "status": "connected",
        "connected_at": "2023-07-19T14:56:51.616329898Z",
        "keepalive_timeout_seconds": 10,
        "reconnect_url": null
      }
    }
  })";

  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::session_welcome);
  EXPECT_EQ(msg.session_id, "AQoQILE98gtqShGmLD7AM6yJThAB");
  EXPECT_EQ(msg.reconnect_url, "");
}

TEST(EventSub, ParsesSessionKeepalive) {
  std::string json = R"({
    "metadata": {
      "message_id": "84c1e79a-2a4b-4c13-ba0b-4312293e9308",
      "message_type": "session_keepalive",
      "message_timestamp": "2023-07-19T10:11:12.634234626Z"
    },
    "payload": {}
  })";

  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::session_keepalive);
}

TEST(EventSub, ParsesSessionReconnectWithUrl) {
  std::string json = R"({
    "metadata": {
      "message_id": "84c1e79a-2a4b-4c13-ba0b-4312293e9308",
      "message_type": "session_reconnect",
      "message_timestamp": "2023-07-19T14:56:51.634234626Z"
    },
    "payload": {
      "session": {
        "id": "AQoQILE98gtqShGmLD7AM6yJThAB",
        "status": "reconnecting",
        "keepalive_timeout_seconds": null,
        "reconnect_url": "wss://eventsub.wss.twitch.tv?%24session_id=abc",
        "connected_at": "2023-07-19T14:56:51.616329898Z"
      }
    }
  })";

  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::session_reconnect);
  EXPECT_EQ(msg.session_id, "AQoQILE98gtqShGmLD7AM6yJThAB");
  EXPECT_EQ(msg.reconnect_url,
            "wss://eventsub.wss.twitch.tv?%24session_id=abc");
}

TEST(EventSub, ParsesChatMessageNotification) {
  std::string json = R"({
    "metadata": {
      "message_id": "9-8-7-6-5",
      "message_type": "notification",
      "message_timestamp": "2023-07-19T14:56:51.634234626Z",
      "subscription_type": "channel.chat.message",
      "subscription_version": "1"
    },
    "payload": {
      "subscription": {
        "id": "abc-123",
        "status": "enabled",
        "type": "channel.chat.message",
        "version": "1",
        "condition": {
          "broadcaster_user_id": "1971641",
          "user_id": "12826"
        },
        "transport": {
          "method": "websocket",
          "session_id": "AQoQILE98gtqShGmLD7AM6yJThAB"
        },
        "created_at": "2023-07-19T14:56:51.616329898Z",
        "cost": 0
      },
      "event": {
        "broadcaster_user_id": "1971641",
        "broadcaster_user_login": "streamer",
        "broadcaster_user_name": "streamer",
        "chatter_user_id": "4145994",
        "chatter_user_login": "viewer32",
        "chatter_user_name": "viewer32",
        "message_id": "cc106a89-1814-919d-454c-f4f2f970aae7",
        "message": {
          "text": "ping",
          "fragments": []
        },
        "message_type": "text"
      }
    }
  })";

  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::notification);
  EXPECT_EQ(msg.subscription_type, "channel.chat.message");
  ASSERT_FALSE(msg.event_json.empty());

  chat_message_t fields = parse_chat_message_event(msg.event);
  EXPECT_EQ(fields.broadcaster.id, "1971641");
  EXPECT_EQ(fields.broadcaster.login, "streamer");
  EXPECT_EQ(fields.broadcaster.name, "streamer");
  EXPECT_EQ(fields.chatter.id, "4145994");
  EXPECT_EQ(fields.chatter.login, "viewer32");
  EXPECT_EQ(fields.chatter.name, "viewer32");
  EXPECT_EQ(fields.message, "ping");
}

TEST(EventSub, ParsesRevocationUsingSubscriptionAsEventJson) {
  std::string json = R"({
    "metadata": {
      "message_id": "84c1e79a-2a4b-4c13-ba0b-4312293e9308",
      "message_type": "revocation",
      "message_timestamp": "2023-07-19T14:56:51.634234626Z",
      "subscription_type": "channel.chat.message",
      "subscription_version": "1"
    },
    "payload": {
      "subscription": {
        "id": "abc-123",
        "status": "authorization_revoked",
        "type": "channel.chat.message",
        "version": "1",
        "condition": {
          "broadcaster_user_id": "1337"
        },
        "transport": {
          "method": "websocket",
          "session_id": "AQoQILE98gtqShGmLD7AM6yJThAB"
        },
        "created_at": "2023-07-19T14:56:51.616329898Z",
        "cost": 0
      }
    }
  })";

  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::revocation);
  EXPECT_EQ(msg.subscription_type, "channel.chat.message");
  EXPECT_NE(msg.event_json.find("authorization_revoked"), std::string::npos);
}

TEST(EventSub, UnknownMessageTypeYieldsUnknown) {
  std::string json =
      R"({"metadata":{"message_type":"something_new"},"payload":{}})";
  eventsub_message msg = parse_eventsub_message(json);
  EXPECT_EQ(msg.type, eventsub_message_type::unknown);
}

TEST(EventSub, GarbageInputDoesNotCrashAndYieldsUnknown) {
  eventsub_message msg = parse_eventsub_message("not even json {{{");
  EXPECT_EQ(msg.type, eventsub_message_type::unknown);
  EXPECT_EQ(msg.session_id, "");
}

TEST(EventSub, ChatMessageFieldsWithMissingFieldsAreEmpty) {
  chat_message_t fields =
      parse_chat_message_event(nlohmann::json::parse(R"({"message":{}})"));
  EXPECT_EQ(fields.broadcaster.id, "");
  EXPECT_EQ(fields.message, "");
}
