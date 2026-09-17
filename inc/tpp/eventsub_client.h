#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "tpp/enums.h"
#include "tpp/event.h"
#include "tpp/event_router.h"
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
 * @brief One parsed EventSub WebSocket message. Internal to
 * eventsub_client, which is the only thing that ever parses one.
 */
struct eventsub_message {
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
 * @brief Payload delivered through eventsub_client::on_welcome.
 */
struct TPP_EXPORT eventsub_welcome_t {
  /**
   * @brief The EventSub WebSocket session ID Twitch assigned this
   * connection, used to patch this shard's transport onto a conduit.
   */
  std::string session_id;
};

/**
 * @brief Payload delivered through eventsub_client::on_notification.
 */
struct TPP_EXPORT eventsub_notification_t {
  /**
   * @brief metadata.subscription_type, e.g. "channel.chat.message"
   */
  std::string subscription_type;

  /**
   * @brief Parsed payload.event.
   */
  nlohmann::json event;

  /**
   * @brief Raw JSON text of payload.event.
   */
  std::string raw;
};

/**
 * @brief Payload delivered through eventsub_client::on_reconnect.
 */
struct TPP_EXPORT eventsub_reconnect_t {
  /**
   * @brief payload.session.reconnect_url to open a new connection to.
   */
  std::string reconnect_url;
};

/**
 * @brief Drives a single EventSub Conduit shard's WebSocket connection,
 * analogous to DPP's dpp::discord_client. Owned by conduit, which pools
 * these into its shards_ vector. Dispatches welcome and reconnect frames
 * to on_welcome/on_reconnect.
 *
 * A notification frame is looked up against the events:: handler
 * registry and handed to event_handler::handle() on this class's own IO
 * thread. on_notification fires for every notification regardless of
 * whether it routed anywhere.
 * @note On a session_reconnect message, this class only reports the new
 * URL via on_reconnect; it does not reconnect itself.
 */
class TPP_EXPORT eventsub_client : public websocket_client {
 public:
  /**
   * @brief The conduit that owns this shard, mirroring DPP's own
   * discord_client::creator.
   */
  conduit *creator;

  /**
   * @brief The shard ID of this connection, mirroring DPP's own
   * discord_client::shard_id. Set by conduit once this shard is opened;
   * used by event_dispatch_t::from() to look the shard back up via
   * conduit::get_shard() rather than storing a pointer that could dangle
   * across a reconnect.
   */
  uint16_t shard_id {0};

  event_router_t<eventsub_welcome_t> on_welcome;
  event_router_t<eventsub_notification_t> on_notification;
  event_router_t<eventsub_reconnect_t> on_reconnect;

  /**
   * @param creator owning conduit
   * @param connect_url full "wss://host[:port]/path" URL to connect to.
   * If empty, connects to Twitch's main EventSub endpoint
   * (wss://eventsub.wss.twitch.tv/ws).
   */
  explicit eventsub_client(conduit *creator, const std::string &connect_url = "");

  virtual bool handle_frame(const std::string &buffer, ws_opcode opcode) override;

  virtual void log(tpp::loglevel severity, const std::string &msg) const override;

 private:
  /**
   * @brief Looks up the handler registered for a notification's
   * subscription type and, if found, hands it this shard - see
   * events::event_handler::handle() for what happens from there.
   */
  void route_notification(const std::string &subscription_type, nlohmann::json event, const std::string &raw);
};

}// namespace tpp
