#pragma once

#include <functional>
#include <string>

#include "tpp/enums.h"
#include "tpp/eventsub.h"
#include "tpp/export.h"
#include "tpp/websocket.h"

namespace tpp {

/**
 * @brief Twitch's EventSub WebSocket endpoint.
 */
static inline constexpr const char *EVENTSUB_HOST = "eventsub.wss.twitch.tv";

/**
 * @brief Path part of Twitch's EventSub WebSocket endpoint.
 */
static inline constexpr const char *EVENTSUB_PATH = "/ws";

using eventsub_welcome_event =
    std::function<void(const std::string &session_id)>;
using eventsub_notification_event = std::function<void(
    const std::string &subscription_type, const std::string &event_json)>;
using eventsub_reconnect_event =
    std::function<void(const std::string &reconnect_url)>;

/**
 * @brief A client for Twitch's EventSub WebSocket protocol. Dispatches
 * parsed frames to on_welcome, on_notification, and on_reconnect.
 * @note On a session_reconnect message, this class only reports the new
 * URL via on_reconnect; it does not reconnect itself.
 */
class TPP_EXPORT eventsub_client : public websocket_client {
 public:
  eventsub_welcome_event      on_welcome {};
  eventsub_notification_event on_notification {};
  eventsub_reconnect_event    on_reconnect {};

  /**
   * @param creator owning application
   * @param connect_url full "wss://host[:port]/path" URL to connect to.
   * If empty, connects to Twitch's main EventSub endpoint
   * (wss://eventsub.wss.twitch.tv/ws).
   */
  explicit eventsub_client(application       *creator,
                           const std::string &connect_url = "");

  virtual bool handle_frame(const std::string &buffer,
                            ws_opcode          opcode) override;

  virtual void log(tpp::loglevel      severity,
                   const std::string &msg) const override;
};

}// namespace tpp
