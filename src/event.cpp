#include <tpp/event.h>

#include <memory>
#include <unordered_map>

namespace tpp {

event event::channel_chat_message(const std::string &user_id) {
  return event {
      "channel.chat.message",
      "1",
      {{"broadcaster_user_id", user_id}, {"user_id", user_id}}
  };
}

}// namespace tpp

namespace tpp::events {

namespace {

const std::unordered_map<std::string, std::unique_ptr<event_handler>> &
registry() {
  static const std::unordered_map<std::string, std::unique_ptr<event_handler>>
      reg = [] {
        std::unordered_map<std::string, std::unique_ptr<event_handler>> m;
        m.emplace("automod.message.hold",
                  std::make_unique<automod_message_hold>());
        m.emplace("automod.message.update",
                  std::make_unique<automod_message_update>());
        m.emplace("automod.settings.update",
                  std::make_unique<automod_settings_update>());
        m.emplace("automod.terms.update",
                  std::make_unique<automod_terms_update>());
        m.emplace("channel.update", std::make_unique<channel_update>());
        m.emplace("channel.follow", std::make_unique<channel_follow>());
        m.emplace("channel.ad_break.begin",
                  std::make_unique<channel_ad_break_begin>());
        m.emplace("channel.chat.clear", std::make_unique<channel_chat_clear>());
        m.emplace("channel.chat.clear_user_messages",
                  std::make_unique<channel_chat_clear_user_messages>());
        m.emplace("channel.chat.message",
                  std::make_unique<channel_chat_message>());
        m.emplace("channel.chat.message_delete",
                  std::make_unique<channel_chat_message_delete>());
        m.emplace("channel.chat.notification",
                  std::make_unique<channel_chat_notification>());
        m.emplace("channel.chat_settings.update",
                  std::make_unique<channel_chat_settings_update>());
        m.emplace("channel.chat.user_message_hold",
                  std::make_unique<channel_chat_user_message_hold>());
        m.emplace("channel.chat.user_message_update",
                  std::make_unique<channel_chat_user_message_update>());
        m.emplace("channel.shared_chat.begin",
                  std::make_unique<channel_shared_chat_begin>());
        m.emplace("channel.shared_chat.update",
                  std::make_unique<channel_shared_chat_update>());
        m.emplace("channel.shared_chat.end",
                  std::make_unique<channel_shared_chat_end>());
        m.emplace("channel.subscribe", std::make_unique<channel_subscribe>());
        m.emplace("channel.subscription.end",
                  std::make_unique<channel_subscription_end>());
        m.emplace("channel.subscription.gift",
                  std::make_unique<channel_subscription_gift>());
        m.emplace("channel.subscription.message",
                  std::make_unique<channel_subscription_message>());
        m.emplace("channel.cheer", std::make_unique<channel_cheer>());
        m.emplace("channel.raid", std::make_unique<channel_raid>());
        m.emplace("channel.ban", std::make_unique<channel_ban>());
        m.emplace("channel.unban", std::make_unique<channel_unban>());
        m.emplace("channel.unban_request.create",
                  std::make_unique<channel_unban_request_create>());
        m.emplace("channel.unban_request.resolve",
                  std::make_unique<channel_unban_request_resolve>());
        m.emplace("channel.moderate", std::make_unique<channel_moderate>());
        m.emplace("channel.moderator.add",
                  std::make_unique<channel_moderator_add>());
        m.emplace("channel.moderator.remove",
                  std::make_unique<channel_moderator_remove>());
        m.emplace("channel.guest_star_session.begin",
                  std::make_unique<channel_guest_star_session_begin>());
        m.emplace("channel.guest_star_session.end",
                  std::make_unique<channel_guest_star_session_end>());
        m.emplace("channel.guest_star_guest.update",
                  std::make_unique<channel_guest_star_guest_update>());
        m.emplace("channel.guest_star_settings.update",
                  std::make_unique<channel_guest_star_settings_update>());
        m.emplace(
            "channel.channel_points_automatic_reward_redemption.add",
            std::make_unique<
                channel_channel_points_automatic_reward_redemption_add>());
        m.emplace("channel.channel_points_custom_reward.add",
                  std::make_unique<channel_channel_points_custom_reward_add>());
        m.emplace(
            "channel.channel_points_custom_reward.update",
            std::make_unique<channel_channel_points_custom_reward_update>());
        m.emplace(
            "channel.channel_points_custom_reward.remove",
            std::make_unique<channel_channel_points_custom_reward_remove>());
        m.emplace("channel.channel_points_custom_reward_redemption.add",
                  std::make_unique<
                      channel_channel_points_custom_reward_redemption_add>());
        m.emplace(
            "channel.channel_points_custom_reward_redemption.update",
            std::make_unique<
                channel_channel_points_custom_reward_redemption_update>());
        m.emplace("channel.poll.begin", std::make_unique<channel_poll_begin>());
        m.emplace("channel.poll.progress",
                  std::make_unique<channel_poll_progress>());
        m.emplace("channel.poll.end", std::make_unique<channel_poll_end>());
        m.emplace("channel.prediction.begin",
                  std::make_unique<channel_prediction_begin>());
        m.emplace("channel.prediction.progress",
                  std::make_unique<channel_prediction_progress>());
        m.emplace("channel.prediction.lock",
                  std::make_unique<channel_prediction_lock>());
        m.emplace("channel.prediction.end",
                  std::make_unique<channel_prediction_end>());
        m.emplace("channel.suspicious_user.message",
                  std::make_unique<channel_suspicious_user_message>());
        m.emplace("channel.suspicious_user.update",
                  std::make_unique<channel_suspicious_user_update>());
        m.emplace("channel.vip.add", std::make_unique<channel_vip_add>());
        m.emplace("channel.vip.remove", std::make_unique<channel_vip_remove>());
        m.emplace("channel.warning.acknowledge",
                  std::make_unique<channel_warning_acknowledge>());
        m.emplace("channel.warning.send",
                  std::make_unique<channel_warning_send>());
        m.emplace("channel.charity_campaign.donate",
                  std::make_unique<channel_charity_campaign_donate>());
        m.emplace("channel.charity_campaign.start",
                  std::make_unique<channel_charity_campaign_start>());
        m.emplace("channel.charity_campaign.progress",
                  std::make_unique<channel_charity_campaign_progress>());
        m.emplace("channel.charity_campaign.stop",
                  std::make_unique<channel_charity_campaign_stop>());
        m.emplace("channel.goal.begin", std::make_unique<channel_goal_begin>());
        m.emplace("channel.goal.progress",
                  std::make_unique<channel_goal_progress>());
        m.emplace("channel.goal.end", std::make_unique<channel_goal_end>());
        m.emplace("channel.hype_train.begin",
                  std::make_unique<channel_hype_train_begin>());
        m.emplace("channel.hype_train.progress",
                  std::make_unique<channel_hype_train_progress>());
        m.emplace("channel.hype_train.end",
                  std::make_unique<channel_hype_train_end>());
        m.emplace("channel.shield_mode.begin",
                  std::make_unique<channel_shield_mode_begin>());
        m.emplace("channel.shield_mode.end",
                  std::make_unique<channel_shield_mode_end>());
        m.emplace("channel.shoutout.create",
                  std::make_unique<channel_shoutout_create>());
        m.emplace("channel.shoutout.receive",
                  std::make_unique<channel_shoutout_receive>());
        m.emplace("conduit.shard.disabled",
                  std::make_unique<conduit_shard_disabled>());
        m.emplace("drop.entitlement.grant",
                  std::make_unique<drop_entitlement_grant>());
        m.emplace("extension.bits_transaction.create",
                  std::make_unique<extension_bits_transaction_create>());
        m.emplace("stream.online", std::make_unique<stream_online>());
        m.emplace("stream.offline", std::make_unique<stream_offline>());
        m.emplace("user.authorization.grant",
                  std::make_unique<user_authorization_grant>());
        m.emplace("user.authorization.revoke",
                  std::make_unique<user_authorization_revoke>());
        m.emplace("user.update", std::make_unique<user_update>());
        m.emplace("user.whisper.message",
                  std::make_unique<user_whisper_message>());
        return m;
      }();
  return reg;
}

}// namespace

const event_handler *find_handler(const std::string &subscription_type) {
  const auto &reg = registry();
  auto        it  = reg.find(subscription_type);
  return it == reg.end() ? nullptr : it->second.get();
}

}// namespace tpp::events
