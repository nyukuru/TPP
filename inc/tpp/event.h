#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "tpp/export.h"

namespace tpp {

class conduit;
class eventsub_client;

/**
 * @brief Describes an EventSub subscription: its type, version, and
 * condition fields, e.g. {"channel.chat.message", "1",
 * {{"broadcaster_user_id", id}, {"user_id", id}}}.
 */
struct TPP_EXPORT event {
  std::string type;
  std::string version {"1"};
  std::map<std::string, std::string> condition;

  /**
   * @brief Builds a "channel.chat.message" subscription for a user's own
   * channel.
   * @param user_id Twitch numeric user ID of the channel/chatter
   */
  static event channel_chat_message(const std::string &user_id);

  static event automod_message_hold(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event automod_message_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event automod_settings_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event automod_terms_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_update(const std::string &broadcaster_user_id);
  static event channel_follow(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_ad_break_begin(const std::string &broadcaster_user_id);
  static event channel_chat_clear(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_clear_user_messages(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_message_delete(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_notification(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_settings_update(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_user_message_hold(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_chat_user_message_update(const std::string &broadcaster_user_id, const std::string &user_id);
  static event channel_shared_chat_begin(const std::string &broadcaster_user_id);
  static event channel_shared_chat_update(const std::string &broadcaster_user_id);
  static event channel_shared_chat_end(const std::string &broadcaster_user_id);
  static event channel_subscribe(const std::string &broadcaster_user_id);
  static event channel_subscription_end(const std::string &broadcaster_user_id);
  static event channel_subscription_gift(const std::string &broadcaster_user_id);
  static event channel_subscription_message(const std::string &broadcaster_user_id);
  static event channel_cheer(const std::string &broadcaster_user_id);
  static event channel_raid(const std::string &to_broadcaster_user_id);
  static event channel_ban(const std::string &broadcaster_user_id);
  static event channel_unban(const std::string &broadcaster_user_id);
  static event channel_unban_request_create(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_unban_request_resolve(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_moderate(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_moderator_add(const std::string &broadcaster_user_id);
  static event channel_moderator_remove(const std::string &broadcaster_user_id);
  static event channel_guest_star_session_begin(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_guest_star_session_end(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_guest_star_guest_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_guest_star_settings_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_channel_points_automatic_reward_redemption_add(const std::string &broadcaster_user_id);
  static event channel_channel_points_custom_reward_add(const std::string &broadcaster_user_id);
  static event channel_channel_points_custom_reward_update(const std::string &broadcaster_user_id, const std::string &reward_id = "");
  static event channel_channel_points_custom_reward_remove(const std::string &broadcaster_user_id, const std::string &reward_id = "");
  static event channel_channel_points_custom_reward_redemption_add(const std::string &broadcaster_user_id, const std::string &reward_id = "");
  static event channel_channel_points_custom_reward_redemption_update(const std::string &broadcaster_user_id, const std::string &reward_id = "");
  static event channel_poll_begin(const std::string &broadcaster_user_id);
  static event channel_poll_progress(const std::string &broadcaster_user_id);
  static event channel_poll_end(const std::string &broadcaster_user_id);
  static event channel_prediction_begin(const std::string &broadcaster_user_id);
  static event channel_prediction_progress(const std::string &broadcaster_user_id);
  static event channel_prediction_lock(const std::string &broadcaster_user_id);
  static event channel_prediction_end(const std::string &broadcaster_user_id);
  static event channel_suspicious_user_message(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_suspicious_user_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_vip_add(const std::string &broadcaster_user_id);
  static event channel_vip_remove(const std::string &broadcaster_user_id);
  static event channel_warning_acknowledge(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_warning_send(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_charity_campaign_donate(const std::string &broadcaster_user_id);
  static event channel_charity_campaign_start(const std::string &broadcaster_user_id);
  static event channel_charity_campaign_progress(const std::string &broadcaster_user_id);
  static event channel_charity_campaign_stop(const std::string &broadcaster_user_id);
  static event channel_goal_begin(const std::string &broadcaster_user_id);
  static event channel_goal_progress(const std::string &broadcaster_user_id);
  static event channel_goal_end(const std::string &broadcaster_user_id);
  static event channel_hype_train_begin(const std::string &broadcaster_user_id);
  static event channel_hype_train_progress(const std::string &broadcaster_user_id);
  static event channel_hype_train_end(const std::string &broadcaster_user_id);
  static event channel_shield_mode_begin(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_shield_mode_end(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_shoutout_create(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event channel_shoutout_receive(const std::string &broadcaster_user_id, const std::string &moderator_user_id);
  static event conduit_shard_disabled(const std::string &client_id, const std::string &conduit_id = "");
  static event drop_entitlement_grant(const std::string &organization_id, const std::string &category_id = "", const std::string &campaign_id = "");
  static event extension_bits_transaction_create(const std::string &extension_client_id);
  static event stream_online(const std::string &broadcaster_user_id);
  static event stream_offline(const std::string &broadcaster_user_id);
  static event user_authorization_grant(const std::string &client_id);
  static event user_authorization_revoke(const std::string &client_id);
  static event user_update(const std::string &user_id);
  static event user_whisper_message(const std::string &user_id);
};

/**
 * @brief Base for event payloads delivered through an event_router_t, e.g.
 * conduit::on_chat_message. Mirrors DPP's own event_dispatch_t, with
 * dpp::cluster swapped for tpp::conduit and dpp::discord_client swapped
 * for tpp::eventsub_client.
 */
struct TPP_EXPORT event_dispatch_t {
  /**
   * @brief Raw JSON text of the event.
   */
  std::string raw_event {};

  /**
   * @brief Shard the event came from.
   */
  uint32_t shard {0};

  /**
   * @brief Conduit owning the event dispatch.
   */
  conduit *owner {nullptr};

  /**
   * @brief Construct a new event_dispatch_t object
   */
  event_dispatch_t() = default;

  /**
   * @brief Construct a new event_dispatch_t object
   * @param rhs event_dispatch_t object to copy from
   */
  event_dispatch_t(const event_dispatch_t &rhs) = default;

  /**
   * @brief Construct a new event_dispatch_t object
   * @param rhs event_dispatch_t object to move from
   */
  event_dispatch_t(event_dispatch_t &&rhs) = default;

  /**
   * @brief Construct a new event_dispatch_t object
   * @param creator The conduit the event originated on.
   * @param shard_id The shard the event originated on.
   * @param raw Raw event data as JSON
   */
  event_dispatch_t(conduit *creator, uint32_t shard_id, const std::string &raw);

  /**
   * @brief Returns the shard object for the event's shard id
   * @return eventsub_client object
   */
  eventsub_client *from() const;

  /**
   * @brief Construct a new event_dispatch_t object
   * @param creator The conduit the event originated on.
   * @param shard_id The shard the event originated on.
   * @param raw Raw event data as JSON
   */
  event_dispatch_t(conduit *creator, uint32_t shard_id, std::string &&raw);

  /**
   * @brief Copy another event_dispatch_t object
   * @param rhs The event to copy from
   */
  event_dispatch_t &operator=(const event_dispatch_t &rhs) = default;

  /**
   * @brief Move from another event_dispatch_t object
   * @param rhs The event to move from
   */
  event_dispatch_t &operator=(event_dispatch_t &&rhs) = default;

  /**
   * @brief Destroy an event_dispatch_t object
   */
  virtual ~event_dispatch_t() = default;
};

/**
 * @brief Type-safe field extraction helpers used by every fill_from_json()
 * method (tpp::user, tpp::message, and the events::event_handler
 * implementations in src/events/ *.cpp) - verbatim copies of DPP's own
 * json_interop.h family (for_each_json()/string_not_null()/
 * int64_not_null()/... and their set_*_not_null() siblings), minus
 * set_iconhash_not_null() (no tpp equivalent of DPP's utility::iconhash).
 * Each getter returns a zero value rather than throwing if j is null,
 * the key is absent, or the value is null/the wrong type - see each
 * function's body for the exact rule, which differs slightly by type
 * (e.g. the numeric getters accept anything but null/string, matching
 * DPP, rather than requiring a strict number type).
 */
TPP_EXPORT void for_each_json(nlohmann::json *parent, std::string_view key, const std::function<void(nlohmann::json *)> &fn);

TPP_EXPORT std::string string_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_string_not_null(const nlohmann::json *j, const char *keyname, std::string &v);

TPP_EXPORT double double_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_double_not_null(const nlohmann::json *j, const char *keyname, double &v);

TPP_EXPORT uint64_t int64_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_int64_not_null(const nlohmann::json *j, const char *keyname, uint64_t &v);

TPP_EXPORT uint32_t int32_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_int32_not_null(const nlohmann::json *j, const char *keyname, uint32_t &v);

TPP_EXPORT uint16_t int16_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_int16_not_null(const nlohmann::json *j, const char *keyname, uint16_t &v);

TPP_EXPORT uint8_t int8_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_int8_not_null(const nlohmann::json *j, const char *keyname, uint8_t &v);

TPP_EXPORT bool bool_not_null(const nlohmann::json *j, const char *keyname);
TPP_EXPORT void set_bool_not_null(const nlohmann::json *j, const char *keyname, bool &v);

/**
 * @brief The events namespace holds the handler for each EventSub
 * notification subscription type.
 */
namespace events {

/**
 * @brief Base class for an EventSub notification handler.
 */
class TPP_EXPORT event_handler {
 public:
  /**
   * @brief Handles one EventSub notification. Mirrors DPP's own event
   * handlers (see dpp::events::message_create::handle in
   * src/dpp/events/message_create.cpp): guards on whether the matching
   * event_router_t has any handlers attached, and if so, queues the rest
   * of the work - parsing the notification, resolving the tracked
   * consumer it belongs to, and firing the router - onto the owning
   * conduit's dispatch thread pool. Nothing above this call does any
   * parsing or consumer lookup; that is this function's job alone.
   * @param client shard the notification arrived on
   * @param j parsed JSON of the notification's payload.event
   * @param raw raw JSON text of the notification's payload.event
   */
  virtual void handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const = 0;

  virtual ~event_handler() = default;
};

/**
 * @brief Declares an EventSub notification handler class. Mirrors DPP's
 * own event_decl macro (see dpp/event.h). Not exported - instances are
 * only ever created inside the library, one static instance per type,
 * by event_map (see src/eventsub_events.cpp).
 */
#define TPP_EVENT_DECL(x)                                                                           \
  class x : public event_handler {                                                                  \
   public:                                                                                          \
    void handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const override; \
  };

TPP_EVENT_DECL(automod_message_hold)
TPP_EVENT_DECL(automod_message_update)
TPP_EVENT_DECL(automod_settings_update)
TPP_EVENT_DECL(automod_terms_update)
TPP_EVENT_DECL(channel_update)
TPP_EVENT_DECL(channel_follow)
TPP_EVENT_DECL(channel_ad_break_begin)
TPP_EVENT_DECL(channel_chat_clear)
TPP_EVENT_DECL(channel_chat_clear_user_messages)
TPP_EVENT_DECL(channel_chat_message)
TPP_EVENT_DECL(channel_chat_message_delete)
TPP_EVENT_DECL(channel_chat_notification)
TPP_EVENT_DECL(channel_chat_settings_update)
TPP_EVENT_DECL(channel_chat_user_message_hold)
TPP_EVENT_DECL(channel_chat_user_message_update)
TPP_EVENT_DECL(channel_shared_chat_begin)
TPP_EVENT_DECL(channel_shared_chat_update)
TPP_EVENT_DECL(channel_shared_chat_end)
TPP_EVENT_DECL(channel_subscribe)
TPP_EVENT_DECL(channel_subscription_end)
TPP_EVENT_DECL(channel_subscription_gift)
TPP_EVENT_DECL(channel_subscription_message)
TPP_EVENT_DECL(channel_cheer)
TPP_EVENT_DECL(channel_raid)
TPP_EVENT_DECL(channel_ban)
TPP_EVENT_DECL(channel_unban)
TPP_EVENT_DECL(channel_unban_request_create)
TPP_EVENT_DECL(channel_unban_request_resolve)
TPP_EVENT_DECL(channel_moderate)
TPP_EVENT_DECL(channel_moderator_add)
TPP_EVENT_DECL(channel_moderator_remove)
TPP_EVENT_DECL(channel_guest_star_session_begin)
TPP_EVENT_DECL(channel_guest_star_session_end)
TPP_EVENT_DECL(channel_guest_star_guest_update)
TPP_EVENT_DECL(channel_guest_star_settings_update)
TPP_EVENT_DECL(channel_channel_points_automatic_reward_redemption_add)
TPP_EVENT_DECL(channel_channel_points_custom_reward_add)
TPP_EVENT_DECL(channel_channel_points_custom_reward_update)
TPP_EVENT_DECL(channel_channel_points_custom_reward_remove)
TPP_EVENT_DECL(channel_channel_points_custom_reward_redemption_add)
TPP_EVENT_DECL(channel_channel_points_custom_reward_redemption_update)
TPP_EVENT_DECL(channel_poll_begin)
TPP_EVENT_DECL(channel_poll_progress)
TPP_EVENT_DECL(channel_poll_end)
TPP_EVENT_DECL(channel_prediction_begin)
TPP_EVENT_DECL(channel_prediction_progress)
TPP_EVENT_DECL(channel_prediction_lock)
TPP_EVENT_DECL(channel_prediction_end)
TPP_EVENT_DECL(channel_suspicious_user_message)
TPP_EVENT_DECL(channel_suspicious_user_update)
TPP_EVENT_DECL(channel_vip_add)
TPP_EVENT_DECL(channel_vip_remove)
TPP_EVENT_DECL(channel_warning_acknowledge)
TPP_EVENT_DECL(channel_warning_send)
TPP_EVENT_DECL(channel_charity_campaign_donate)
TPP_EVENT_DECL(channel_charity_campaign_start)
TPP_EVENT_DECL(channel_charity_campaign_progress)
TPP_EVENT_DECL(channel_charity_campaign_stop)
TPP_EVENT_DECL(channel_goal_begin)
TPP_EVENT_DECL(channel_goal_progress)
TPP_EVENT_DECL(channel_goal_end)
TPP_EVENT_DECL(channel_hype_train_begin)
TPP_EVENT_DECL(channel_hype_train_progress)
TPP_EVENT_DECL(channel_hype_train_end)
TPP_EVENT_DECL(channel_shield_mode_begin)
TPP_EVENT_DECL(channel_shield_mode_end)
TPP_EVENT_DECL(channel_shoutout_create)
TPP_EVENT_DECL(channel_shoutout_receive)
TPP_EVENT_DECL(conduit_shard_disabled)
TPP_EVENT_DECL(drop_entitlement_grant)
TPP_EVENT_DECL(extension_bits_transaction_create)
TPP_EVENT_DECL(stream_online)
TPP_EVENT_DECL(stream_offline)
TPP_EVENT_DECL(user_authorization_grant)
TPP_EVENT_DECL(user_authorization_revoke)
TPP_EVENT_DECL(user_update)
TPP_EVENT_DECL(user_whisper_message)

#undef TPP_EVENT_DECL

/**
 * @brief Looks up and invokes the handler registered for a notification's
 * subscription type, mirroring DPP's discord_client::handle_event. Logs
 * at ll_debug and does nothing else if subscription_type has no
 * registered handler.
 * @param client shard the notification arrived on
 * @param subscription_type e.g. "channel.chat.message"
 * @param j parsed JSON of the notification's payload.event
 * @param raw raw JSON text of the notification's payload.event
 */
TPP_EXPORT void handle_event(eventsub_client *client, const std::string &subscription_type, nlohmann::json &j, const std::string &raw);

/**
 * @brief Looks up the handler registered for a subscription type, without
 * invoking it.
 * @param subscription_type e.g. "channel.chat.message"
 * @return the handler, or nullptr if subscription_type has no registered
 * handler
 */
TPP_EXPORT const event_handler *find_handler(const std::string &subscription_type);

}// namespace events
}// namespace tpp
