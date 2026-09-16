#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"

namespace tpp {

class consumer;

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
};

/**
 * @brief Base for event payloads delivered through an event_router_t,
 * e.g. conduit::on_chat_message.
 */
struct TPP_EXPORT event_dispatch_t {
  /**
   * @brief The consumer the event arrived for.
   */
  consumer *from {nullptr};

  /**
   * @brief Raw JSON text of the event.
   */
  std::string raw_event;
};

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
   * @brief Handles one EventSub notification.
   * @param c consumer the notification was resolved for
   * @param j parsed JSON of the notification's payload.event
   * @param raw raw JSON text of the notification's payload.event
   */
  virtual void handle(consumer *c, nlohmann::json &j, const std::string &raw) const = 0;

  virtual ~event_handler() = default;
};

/**
 * @brief Declares an EventSub notification handler class. Mirrors DPP's
 * own event_decl macro (see dpp/event.h). Not exported - instances are
 * only ever created inside the library, one static instance per type,
 * by event_map (see src/eventsub_events.cpp).
 */
#define TPP_EVENT_DECL(x)                                                               \
  class x : public event_handler {                                                      \
   public:                                                                              \
    void handle(consumer *c, nlohmann::json &j, const std::string &raw) const override; \
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
 * @param c consumer the notification was resolved for
 * @param subscription_type e.g. "channel.chat.message"
 * @param j parsed JSON of the notification's payload.event
 * @param raw raw JSON text of the notification's payload.event
 */
TPP_EXPORT void handle_event(consumer *c, const std::string &subscription_type, nlohmann::json &j, const std::string &raw);

}// namespace events
}// namespace tpp
