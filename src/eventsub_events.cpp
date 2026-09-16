#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>

#include <map>
#include <string>

namespace tpp {

event event::channel_chat_message(const std::string &user_id) {
  return event {"channel.chat.message", "1", {{"broadcaster_user_id", user_id}, {"user_id", user_id}}};
}

/**
 * @brief Constructs one static, stateless instance of an event handler
 * type and returns a pointer to it - mirrors DPP's own make_static_event
 * helper. Handlers carry no state, so one shared instance per type,
 * living for the process's lifetime, is all event_map needs.
 */
template<typename T>
events::event_handler *make_static_event() {
  static T static_event;
  return &static_event;
}

/**
 * @brief Maps every known EventSub subscription type to its handler.
 * Mirrors DPP's own event_map (see dpp::discord_client::handle_event).
 */
const std::map<std::string, events::event_handler *> event_map = {
    {"automod.message.hold", make_static_event<events::automod_message_hold>()},
    {"automod.message.update", make_static_event<events::automod_message_update>()},
    {"automod.settings.update", make_static_event<events::automod_settings_update>()},
    {"automod.terms.update", make_static_event<events::automod_terms_update>()},
    {"channel.update", make_static_event<events::channel_update>()},
    {"channel.follow", make_static_event<events::channel_follow>()},
    {"channel.ad_break.begin", make_static_event<events::channel_ad_break_begin>()},
    {"channel.chat.clear", make_static_event<events::channel_chat_clear>()},
    {"channel.chat.clear_user_messages", make_static_event<events::channel_chat_clear_user_messages>()},
    {"channel.chat.message", make_static_event<events::channel_chat_message>()},
    {"channel.chat.message_delete", make_static_event<events::channel_chat_message_delete>()},
    {"channel.chat.notification", make_static_event<events::channel_chat_notification>()},
    {"channel.chat_settings.update", make_static_event<events::channel_chat_settings_update>()},
    {"channel.chat.user_message_hold", make_static_event<events::channel_chat_user_message_hold>()},
    {"channel.chat.user_message_update", make_static_event<events::channel_chat_user_message_update>()},
    {"channel.shared_chat.begin", make_static_event<events::channel_shared_chat_begin>()},
    {"channel.shared_chat.update", make_static_event<events::channel_shared_chat_update>()},
    {"channel.shared_chat.end", make_static_event<events::channel_shared_chat_end>()},
    {"channel.subscribe", make_static_event<events::channel_subscribe>()},
    {"channel.subscription.end", make_static_event<events::channel_subscription_end>()},
    {"channel.subscription.gift", make_static_event<events::channel_subscription_gift>()},
    {"channel.subscription.message", make_static_event<events::channel_subscription_message>()},
    {"channel.cheer", make_static_event<events::channel_cheer>()},
    {"channel.raid", make_static_event<events::channel_raid>()},
    {"channel.ban", make_static_event<events::channel_ban>()},
    {"channel.unban", make_static_event<events::channel_unban>()},
    {"channel.unban_request.create", make_static_event<events::channel_unban_request_create>()},
    {"channel.unban_request.resolve", make_static_event<events::channel_unban_request_resolve>()},
    {"channel.moderate", make_static_event<events::channel_moderate>()},
    {"channel.moderator.add", make_static_event<events::channel_moderator_add>()},
    {"channel.moderator.remove", make_static_event<events::channel_moderator_remove>()},
    {"channel.guest_star_session.begin", make_static_event<events::channel_guest_star_session_begin>()},
    {"channel.guest_star_session.end", make_static_event<events::channel_guest_star_session_end>()},
    {"channel.guest_star_guest.update", make_static_event<events::channel_guest_star_guest_update>()},
    {"channel.guest_star_settings.update", make_static_event<events::channel_guest_star_settings_update>()},
    {"channel.channel_points_automatic_reward_redemption.add", make_static_event<events::channel_channel_points_automatic_reward_redemption_add>()},
    {"channel.channel_points_custom_reward.add", make_static_event<events::channel_channel_points_custom_reward_add>()},
    {"channel.channel_points_custom_reward.update", make_static_event<events::channel_channel_points_custom_reward_update>()},
    {"channel.channel_points_custom_reward.remove", make_static_event<events::channel_channel_points_custom_reward_remove>()},
    {"channel.channel_points_custom_reward_redemption.add", make_static_event<events::channel_channel_points_custom_reward_redemption_add>()},
    {"channel.channel_points_custom_reward_redemption.update", make_static_event<events::channel_channel_points_custom_reward_redemption_update>()},
    {"channel.poll.begin", make_static_event<events::channel_poll_begin>()},
    {"channel.poll.progress", make_static_event<events::channel_poll_progress>()},
    {"channel.poll.end", make_static_event<events::channel_poll_end>()},
    {"channel.prediction.begin", make_static_event<events::channel_prediction_begin>()},
    {"channel.prediction.progress", make_static_event<events::channel_prediction_progress>()},
    {"channel.prediction.lock", make_static_event<events::channel_prediction_lock>()},
    {"channel.prediction.end", make_static_event<events::channel_prediction_end>()},
    {"channel.suspicious_user.message", make_static_event<events::channel_suspicious_user_message>()},
    {"channel.suspicious_user.update", make_static_event<events::channel_suspicious_user_update>()},
    {"channel.vip.add", make_static_event<events::channel_vip_add>()},
    {"channel.vip.remove", make_static_event<events::channel_vip_remove>()},
    {"channel.warning.acknowledge", make_static_event<events::channel_warning_acknowledge>()},
    {"channel.warning.send", make_static_event<events::channel_warning_send>()},
    {"channel.charity_campaign.donate", make_static_event<events::channel_charity_campaign_donate>()},
    {"channel.charity_campaign.start", make_static_event<events::channel_charity_campaign_start>()},
    {"channel.charity_campaign.progress", make_static_event<events::channel_charity_campaign_progress>()},
    {"channel.charity_campaign.stop", make_static_event<events::channel_charity_campaign_stop>()},
    {"channel.goal.begin", make_static_event<events::channel_goal_begin>()},
    {"channel.goal.progress", make_static_event<events::channel_goal_progress>()},
    {"channel.goal.end", make_static_event<events::channel_goal_end>()},
    {"channel.hype_train.begin", make_static_event<events::channel_hype_train_begin>()},
    {"channel.hype_train.progress", make_static_event<events::channel_hype_train_progress>()},
    {"channel.hype_train.end", make_static_event<events::channel_hype_train_end>()},
    {"channel.shield_mode.begin", make_static_event<events::channel_shield_mode_begin>()},
    {"channel.shield_mode.end", make_static_event<events::channel_shield_mode_end>()},
    {"channel.shoutout.create", make_static_event<events::channel_shoutout_create>()},
    {"channel.shoutout.receive", make_static_event<events::channel_shoutout_receive>()},
    {"conduit.shard.disabled", make_static_event<events::conduit_shard_disabled>()},
    {"drop.entitlement.grant", make_static_event<events::drop_entitlement_grant>()},
    {"extension.bits_transaction.create", make_static_event<events::extension_bits_transaction_create>()},
    {"stream.online", make_static_event<events::stream_online>()},
    {"stream.offline", make_static_event<events::stream_offline>()},
    {"user.authorization.grant", make_static_event<events::user_authorization_grant>()},
    {"user.authorization.revoke", make_static_event<events::user_authorization_revoke>()},
    {"user.update", make_static_event<events::user_update>()},
    {"user.whisper.message", make_static_event<events::user_whisper_message>()},
};

void events::handle_event(consumer *c, const std::string &subscription_type, nlohmann::json &j, const std::string &raw) {
  auto it = event_map.find(subscription_type);
  if (it != event_map.end()) {
    /* A handler with nullptr would be silently ignored, same as DPP -
     * event_map has none today since every subscription type TPP knows
     * about already has a (possibly no-op) handler class, but a future
     * type added to the map before its handler exists could use one. */
    if (it->second != nullptr) {
      it->second->handle(c, j, raw);
    }
  } else {
    c->get_conduit().log(ll_debug, "Unhandled event: " + subscription_type + ", " + j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
  }
}

}// namespace tpp
