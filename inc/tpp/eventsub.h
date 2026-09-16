#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "tpp/event.h"
#include "tpp/export.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief The "metadata.message_type" of an EventSub WebSocket message.
 * @see https://dev.twitch.tv/docs/eventsub/handling-websocket-events/
 */
enum class eventsub_message_type {
  unknown,
  session_welcome,
  session_keepalive,
  session_reconnect,
  notification,
  revocation,
};

/**
 * @brief One parsed EventSub WebSocket message.
 */
struct TPP_EXPORT eventsub_message {
  eventsub_message_type type {eventsub_message_type::unknown};

  /**
   * @brief payload.session.id - present on session_welcome and
   * session_reconnect
   */
  std::string session_id;

  /**
   * @brief payload.session.reconnect_url - present on session_reconnect
   */
  std::string reconnect_url;

  /**
   * @brief metadata.subscription_type - present on notification and
   * revocation
   */
  std::string subscription_type;

  /**
   * @brief Parsed payload.event (notification) or payload.subscription
   * (revocation).
   */
  nlohmann::json event;

  /**
   * @brief Raw JSON text of payload.event (notification) or
   * payload.subscription (revocation).
   */
  std::string event_json;
};

/**
 * @brief Parses a single EventSub WebSocket message.
 * @param json raw JSON text of one EventSub WebSocket text frame
 * @return parsed message; type is eventsub_message_type::unknown if json
 * could not be recognised as an EventSub message
 */
TPP_EXPORT eventsub_message parse_eventsub_message(const std::string &json);

/**
 * @brief Payload delivered through session::on_chat_message for a
 * "channel.chat.message" notification.
 */
struct TPP_EXPORT chat_message_t : public event_dispatch_t {
  user        broadcaster;
  user        chatter;
  std::string message;
};

/**
 * @brief Parses "channel.chat.message" fields from a notification's event
 * data. Does not set chat_message_t::from or ::raw_event.
 * @param event_data eventsub_message::event from a notification whose
 * subscription_type is "channel.chat.message"
 */
TPP_EXPORT chat_message_t
parse_chat_message_event(const nlohmann::json &event_data);

}// namespace tpp
