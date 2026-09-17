#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "tpp/automod.h"
#include "tpp/channel_points.h"
#include "tpp/chat_notification.h"
#include "tpp/event.h"
#include "tpp/eventsub_transport.h"
#include "tpp/export.h"
#include "tpp/hype_train.h"
#include "tpp/message.h"
#include "tpp/moderate.h"
#include "tpp/poll.h"
#include "tpp/prediction.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief Payload delivered through conduit::on_chat_message for a
 * "channel.chat.message" notification.
 */
struct TPP_EXPORT chat_message_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  message msg;
};

/**
 * @brief Payload delivered through conduit::on_automod_message_hold for an
 * "automod.message.hold" notification.
 */
struct TPP_EXPORT automod_message_hold_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  /**
   * @brief msg.chatter is keyed "user_id"/"user_login"/"user_name" for
   * this event, not "chatter_user_*" - see message::fill_from_json.
   */
  message msg;
  std::string category;
  int level {0};
  std::string held_at;
  std::string reason;
  automod_check_result automod;
  blocked_term_check_result blocked_term;
};

/**
 * @brief Payload delivered through conduit::on_automod_message_update for
 * an "automod.message.update" notification.
 */
struct TPP_EXPORT automod_message_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  /**
   * @brief msg.chatter is keyed "user_id"/"user_login"/"user_name" for
   * this event, not "chatter_user_*" - see message::fill_from_json.
   */
  message msg;
  user moderator;
  std::string status;
  std::string category;
  int level {0};
  std::string held_at;
  std::string reason;
  automod_check_result automod;
  blocked_term_check_result blocked_term;
};

/**
 * @brief Payload delivered through conduit::on_automod_settings_update for
 * an "automod.settings.update" notification.
 */
struct TPP_EXPORT automod_settings_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  int overall_level {0};
  int bullying {0};
  int disability {0};
  int race_ethnicity_or_religion {0};
  int misogyny {0};
  int sexuality_sex_or_gender {0};
  int aggression {0};
  int sex_based_terms {0};
  int swearing {0};
};

/**
 * @brief Payload delivered through conduit::on_automod_terms_update for an
 * "automod.terms.update" notification.
 */
struct TPP_EXPORT automod_terms_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  std::string action;
  bool from_automod {false};
  std::vector<std::string> terms;
};

/**
 * @brief Payload delivered through conduit::on_channel_update for a
 * "channel.update" notification.
 */
struct TPP_EXPORT channel_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string title;
  std::string language;
  std::string category_id;
  std::string category_name;
  std::vector<std::string> content_classification_labels;
};

/**
 * @brief Payload delivered through conduit::on_channel_follow for a
 * "channel.follow" notification.
 */
struct TPP_EXPORT channel_follow_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user follower;
  std::string followed_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_ad_break_begin for
 * a "channel.ad_break.begin" notification.
 */
struct TPP_EXPORT channel_ad_break_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user requester;
  int64_t duration_seconds {0};
  std::string started_at;
  bool is_automatic {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_chat_clear for a
 * "channel.chat.clear" notification.
 */
struct TPP_EXPORT channel_chat_clear_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_chat_clear_user_messages for a
 * "channel.chat.clear_user_messages" notification.
 */
struct TPP_EXPORT channel_chat_clear_user_messages_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
};

/**
 * @brief Payload delivered through conduit::on_channel_chat_message_delete
 * for a "channel.chat.message_delete" notification.
 */
struct TPP_EXPORT channel_chat_message_delete_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
  std::string message_id;
};

/**
 * @brief Payload delivered through conduit::on_channel_chat_notification
 * for a "channel.chat.notification" notification.
 */
struct TPP_EXPORT channel_chat_notification_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  message msg;
  bool chatter_is_anonymous {false};
  std::string color;
  std::vector<chat_badge> badges;
  std::string system_message;
  /**
   * @brief One of "sub", "resub", "sub_gift", "community_sub_gift",
   * "gift_paid_upgrade", "prime_paid_upgrade", "pay_it_forward", "raid",
   * "unraid", "announcement", "bits_badge_tier", "charity_donation",
   * "shared_chat_sub", "shared_chat_resub", ... - see notice_type.
   */
  std::string notice_type;
  chat_notice_metadata notice_metadata;
};

/**
 * @brief Payload delivered through conduit::on_channel_chat_settings_update
 * for a "channel.chat_settings.update" notification.
 */
struct TPP_EXPORT channel_chat_settings_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  bool emote_mode {false};
  bool follower_mode {false};
  int64_t follower_mode_duration_minutes {0};
  bool slow_mode {false};
  int64_t slow_mode_wait_time_seconds {0};
  bool subscriber_mode {false};
  bool unique_chat_mode {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_chat_user_message_hold
 * for a "channel.chat.user_message_hold" notification.
 */
struct TPP_EXPORT channel_chat_user_message_hold_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  /**
   * @brief msg.chatter is keyed "user_id"/"user_login"/"user_name" for
   * this event, not "chatter_user_*" - see message::fill_from_json.
   */
  message msg;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_chat_user_message_update for a
 * "channel.chat.user_message_update" notification.
 */
struct TPP_EXPORT channel_chat_user_message_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  /**
   * @brief msg.chatter is keyed "user_id"/"user_login"/"user_name" for
   * this event, not "chatter_user_*" - see message::fill_from_json.
   */
  message msg;
  std::string status;
};

/**
 * @brief Payload delivered through conduit::on_channel_shared_chat_begin
 * for a "channel.shared_chat.begin" notification.
 */
struct TPP_EXPORT channel_shared_chat_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  user host_broadcaster;
  /**
   * @brief Each element's user is keyed "broadcaster_user_id"/"_login"/
   * "_name" within its own object.
   */
  std::vector<user> participants;
};

/**
 * @brief Payload delivered through conduit::on_channel_shared_chat_update
 * for a "channel.shared_chat.update" notification.
 */
struct TPP_EXPORT channel_shared_chat_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  user host_broadcaster;
  /**
   * @brief Each element's user is keyed "broadcaster_user_id"/"_login"/
   * "_name" within its own object.
   */
  std::vector<user> participants;
};

/**
 * @brief Payload delivered through conduit::on_channel_shared_chat_end for
 * a "channel.shared_chat.end" notification.
 */
struct TPP_EXPORT channel_shared_chat_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  user host_broadcaster;
};

/**
 * @brief Payload delivered through conduit::on_channel_subscribe for a
 * "channel.subscribe" notification.
 */
struct TPP_EXPORT channel_subscribe_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user subscriber;
  std::string tier;
  bool is_gift {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_subscription_end for
 * a "channel.subscription.end" notification.
 */
struct TPP_EXPORT channel_subscription_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user subscriber;
  std::string tier;
  bool is_gift {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_subscription_gift
 * for a "channel.subscription.gift" notification.
 */
struct TPP_EXPORT channel_subscription_gift_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user gifter;
  bool is_anonymous {false};
  int64_t total {0};
  std::string tier;
  int64_t cumulative_total {0};
  bool cumulative_total_is_null {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_subscription_message
 * for a "channel.subscription.message" notification.
 */
struct TPP_EXPORT channel_subscription_message_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  /**
   * @brief msg.chatter (the subscriber) is keyed "user_id"/"user_login"/
   * "user_name"; msg.id is always empty, this event has no message id.
   */
  message msg;
  std::string tier;
  int64_t cumulative_months {0};
  int64_t streak_months {0};
  bool streak_months_is_null {false};
  int64_t duration_months {0};
};

/**
 * @brief Payload delivered through conduit::on_channel_cheer for a
 * "channel.cheer" notification.
 */
struct TPP_EXPORT channel_cheer_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user chatter;
  bool is_anonymous {false};
  std::string message;
  int64_t bits {0};
};

/**
 * @brief Payload delivered through conduit::on_channel_raid for a
 * "channel.raid" notification.
 */
struct TPP_EXPORT channel_raid_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user from_broadcaster;
  user to_broadcaster;
  int64_t viewers {0};
};

/**
 * @brief Payload delivered through conduit::on_channel_ban for a
 * "channel.ban" notification.
 */
struct TPP_EXPORT channel_ban_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
  user moderator;
  std::string reason;
  std::string banned_at;
  std::string ends_at;
  bool is_permanent {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_unban for a
 * "channel.unban" notification.
 */
struct TPP_EXPORT channel_unban_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
  user moderator;
};

/**
 * @brief Payload delivered through conduit::on_channel_unban_request_create
 * for a "channel.unban_request.create" notification.
 */
struct TPP_EXPORT channel_unban_request_create_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  user target;
  std::string text;
  std::string created_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_unban_request_resolve for a
 * "channel.unban_request.resolve" notification.
 */
struct TPP_EXPORT channel_unban_request_resolve_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  user moderator;
  user target;
  std::string resolution_text;
  std::string status;
};

/**
 * @brief Payload delivered through conduit::on_channel_moderate for a
 * "channel.moderate" notification.
 */
struct TPP_EXPORT channel_moderate_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user source_broadcaster;
  user moderator;
  /**
   * @brief One of "ban", "timeout", "unban", "untimeout", "clear",
   * "emoteonly", ..., "delete", "vip", "unvip", "mod", "unmod", "raid",
   * "unraid", "followers", "followersoff", "slow", "slowoff",
   * "subscribers", "subscribersoff", "uniquechat", "uniquechatoff",
   * "warn", "shared_chat_ban", "shared_chat_timeout",
   * "shared_chat_unban", "shared_chat_untimeout", "shared_chat_delete".
   */
  std::string action;
  channel_moderate_action_metadata action_metadata;
};

/**
 * @brief Payload delivered through conduit::on_channel_moderator_add for a
 * "channel.moderator.add" notification.
 */
struct TPP_EXPORT channel_moderator_add_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
};

/**
 * @brief Payload delivered through conduit::on_channel_moderator_remove
 * for a "channel.moderator.remove" notification.
 */
struct TPP_EXPORT channel_moderator_remove_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_guest_star_session_begin for a
 * "channel.guest_star_session.begin" notification.
 */
struct TPP_EXPORT channel_guest_star_session_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  std::string started_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_guest_star_session_end for a
 * "channel.guest_star_session.end" notification.
 */
struct TPP_EXPORT channel_guest_star_session_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  std::string started_at;
  std::string ended_at;
  user host;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_guest_star_guest_update for a
 * "channel.guest_star_guest.update" notification.
 */
struct TPP_EXPORT channel_guest_star_guest_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  std::string session_id;
  user moderator;
  user guest;
  std::string slot_id;
  std::string state;
  user host;
  bool host_video_enabled {false};
  bool host_audio_enabled {false};
  int64_t host_volume {0};
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_guest_star_settings_update for a
 * "channel.guest_star_settings.update" notification.
 */
struct TPP_EXPORT channel_guest_star_settings_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  bool is_moderator_send_live_enabled {false};
  int64_t slot_count {0};
  bool is_browser_source_audio_enabled {false};
  std::string group_layout;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_automatic_reward_redemption_add for a
 * "channel.channel_points_automatic_reward_redemption.add" notification.
 */
struct TPP_EXPORT channel_channel_points_automatic_reward_redemption_add_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user redeemer;
  std::string reward_id;
  std::string reward_title;
  std::string reward_prompt;
  int64_t reward_cost {0};
  std::string user_input;
  std::string redeemed_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_custom_reward_add for a
 * "channel.channel_points_custom_reward.add" notification.
 */
struct TPP_EXPORT channel_channel_points_custom_reward_add_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  bool is_enabled {true};
  bool is_paused {false};
  bool is_in_stock {true};
  std::string title;
  int64_t cost {0};
  std::string prompt;
  bool is_user_input_required {false};
  bool should_redemptions_skip_request_queue {false};
  std::string cooldown_expires_at;
  int64_t redemptions_redeemed_current_stream {0};
  reward_limit_setting max_per_stream;
  reward_limit_setting max_per_user_per_stream;
  reward_cooldown_setting global_cooldown;
  std::string background_color;
  reward_image image;
  reward_image default_image;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_custom_reward_update for a
 * "channel.channel_points_custom_reward.update" notification.
 */
struct TPP_EXPORT channel_channel_points_custom_reward_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  bool is_enabled {true};
  bool is_paused {false};
  bool is_in_stock {true};
  std::string title;
  int64_t cost {0};
  std::string prompt;
  bool is_user_input_required {false};
  bool should_redemptions_skip_request_queue {false};
  std::string cooldown_expires_at;
  int64_t redemptions_redeemed_current_stream {0};
  reward_limit_setting max_per_stream;
  reward_limit_setting max_per_user_per_stream;
  reward_cooldown_setting global_cooldown;
  std::string background_color;
  reward_image image;
  reward_image default_image;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_custom_reward_remove for a
 * "channel.channel_points_custom_reward.remove" notification.
 */
struct TPP_EXPORT channel_channel_points_custom_reward_remove_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  bool is_enabled {true};
  bool is_paused {false};
  bool is_in_stock {true};
  std::string title;
  int64_t cost {0};
  std::string prompt;
  bool is_user_input_required {false};
  bool should_redemptions_skip_request_queue {false};
  std::string cooldown_expires_at;
  int64_t redemptions_redeemed_current_stream {0};
  reward_limit_setting max_per_stream;
  reward_limit_setting max_per_user_per_stream;
  reward_cooldown_setting global_cooldown;
  std::string background_color;
  reward_image image;
  reward_image default_image;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_custom_reward_redemption_add for a
 * "channel.channel_points_custom_reward_redemption.add" notification.
 */
struct TPP_EXPORT channel_channel_points_custom_reward_redemption_add_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  user redeemer;
  std::string user_input;
  std::string status;
  redemption_reward reward;
  std::string redeemed_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_channel_points_custom_reward_redemption_update for a
 * "channel.channel_points_custom_reward_redemption.update" notification.
 */
struct TPP_EXPORT channel_channel_points_custom_reward_redemption_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  user redeemer;
  std::string user_input;
  std::string status;
  redemption_reward reward;
  std::string redeemed_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_poll_begin for a
 * "channel.poll.begin" notification.
 */
struct TPP_EXPORT channel_poll_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<poll_choice> choices;
  poll_voting bits_voting;
  poll_voting channel_points_voting;
  std::string started_at;
  std::string ends_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_poll_progress for a
 * "channel.poll.progress" notification.
 */
struct TPP_EXPORT channel_poll_progress_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<poll_choice> choices;
  poll_voting bits_voting;
  poll_voting channel_points_voting;
  std::string started_at;
  std::string ends_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_poll_end for a
 * "channel.poll.end" notification.
 */
struct TPP_EXPORT channel_poll_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<poll_choice> choices;
  poll_voting bits_voting;
  poll_voting channel_points_voting;
  std::string started_at;
  std::string ended_at;
  std::string status;
};

/**
 * @brief Payload delivered through conduit::on_channel_prediction_begin for
 * a "channel.prediction.begin" notification.
 */
struct TPP_EXPORT channel_prediction_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<prediction_outcome> outcomes;
  int64_t prediction_window_seconds {0};
  std::string created_at;
  std::string locks_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_prediction_progress
 * for a "channel.prediction.progress" notification.
 */
struct TPP_EXPORT channel_prediction_progress_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<prediction_outcome> outcomes;
  int64_t prediction_window_seconds {0};
  std::string created_at;
  std::string locks_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_prediction_lock for
 * a "channel.prediction.lock" notification.
 */
struct TPP_EXPORT channel_prediction_lock_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::vector<prediction_outcome> outcomes;
  int64_t prediction_window_seconds {0};
  std::string created_at;
  std::string locks_at;
  std::string locked_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_prediction_end for
 * a "channel.prediction.end" notification.
 */
struct TPP_EXPORT channel_prediction_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string title;
  std::string winning_outcome_id;
  std::vector<prediction_outcome> outcomes;
  std::string created_at;
  std::string ended_at;
  std::string ended_reason;
};

/**
 * @brief Payload delivered through conduit::on_channel_suspicious_user_message
 * for a "channel.suspicious_user.message" notification.
 */
struct TPP_EXPORT channel_suspicious_user_message_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
  std::string low_trust_status;
  std::vector<std::string> shared_ban_channel_ids;
  std::vector<std::string> types;
  std::string ban_evasion_evaluation;
  std::string message_id;
  std::string message_text;
  std::vector<chat_fragment> message_fragments;
};

/**
 * @brief Payload delivered through conduit::on_channel_suspicious_user_update
 * for a "channel.suspicious_user.update" notification.
 */
struct TPP_EXPORT channel_suspicious_user_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  user target;
  std::string low_trust_status;
};

/**
 * @brief Payload delivered through conduit::on_channel_vip_add for a
 * "channel.vip.add" notification.
 */
struct TPP_EXPORT channel_vip_add_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
};

/**
 * @brief Payload delivered through conduit::on_channel_vip_remove for a
 * "channel.vip.remove" notification.
 */
struct TPP_EXPORT channel_vip_remove_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
};

/**
 * @brief Payload delivered through conduit::on_channel_warning_acknowledge
 * for a "channel.warning.acknowledge" notification.
 */
struct TPP_EXPORT channel_warning_acknowledge_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user target;
};

/**
 * @brief Payload delivered through conduit::on_channel_warning_send for a
 * "channel.warning.send" notification.
 */
struct TPP_EXPORT channel_warning_send_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  user target;
  std::string reason;
  std::vector<std::string> chat_rules_cited;
};

/**
 * @brief Payload delivered through conduit::on_channel_charity_campaign_donate
 * for a "channel.charity_campaign.donate" notification.
 */
struct TPP_EXPORT channel_charity_campaign_donate_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  std::string campaign_id;
  user broadcaster;
  user donor;
  std::string charity_name;
  std::string charity_description;
  std::string charity_logo;
  std::string charity_website;
  int64_t amount_value {0};
  int64_t amount_decimal_places {0};
  std::string amount_currency;
};

/**
 * @brief Payload delivered through conduit::on_channel_charity_campaign_start
 * for a "channel.charity_campaign.start" notification.
 */
struct TPP_EXPORT channel_charity_campaign_start_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string charity_name;
  std::string charity_description;
  std::string charity_logo;
  std::string charity_website;
  int64_t current_amount_value {0};
  int64_t current_amount_decimal_places {0};
  std::string current_amount_currency;
  int64_t target_amount_value {0};
  int64_t target_amount_decimal_places {0};
  std::string target_amount_currency;
  std::string started_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_channel_charity_campaign_progress for a
 * "channel.charity_campaign.progress" notification.
 */
struct TPP_EXPORT channel_charity_campaign_progress_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string charity_name;
  std::string charity_description;
  std::string charity_logo;
  std::string charity_website;
  int64_t current_amount_value {0};
  int64_t current_amount_decimal_places {0};
  std::string current_amount_currency;
  int64_t target_amount_value {0};
  int64_t target_amount_decimal_places {0};
  std::string target_amount_currency;
};

/**
 * @brief Payload delivered through conduit::on_channel_charity_campaign_stop
 * for a "channel.charity_campaign.stop" notification.
 */
struct TPP_EXPORT channel_charity_campaign_stop_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string charity_name;
  std::string charity_description;
  std::string charity_logo;
  std::string charity_website;
  int64_t current_amount_value {0};
  int64_t current_amount_decimal_places {0};
  std::string current_amount_currency;
  int64_t target_amount_value {0};
  int64_t target_amount_decimal_places {0};
  std::string target_amount_currency;
  std::string stopped_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_goal_begin for a
 * "channel.goal.begin" notification.
 */
struct TPP_EXPORT channel_goal_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string type;
  std::string description;
  int64_t current_amount {0};
  int64_t target_amount {0};
  std::string started_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_goal_progress for a
 * "channel.goal.progress" notification.
 */
struct TPP_EXPORT channel_goal_progress_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string type;
  std::string description;
  int64_t current_amount {0};
  int64_t target_amount {0};
  std::string started_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_goal_end for a
 * "channel.goal.end" notification.
 */
struct TPP_EXPORT channel_goal_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  std::string type;
  std::string description;
  int64_t current_amount {0};
  int64_t target_amount {0};
  std::string started_at;
  std::string ended_at;
  bool is_achieved {false};
};

/**
 * @brief Payload delivered through conduit::on_channel_hype_train_begin for
 * a "channel.hype_train.begin" notification.
 */
struct TPP_EXPORT channel_hype_train_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  int64_t total {0};
  int64_t progress {0};
  int64_t goal {0};
  std::vector<hype_train_contribution> top_contributions;
  hype_train_contribution last_contribution;
  int64_t level {0};
  bool is_shared_train {false};
  std::string started_at;
  std::string expires_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_hype_train_progress
 * for a "channel.hype_train.progress" notification.
 */
struct TPP_EXPORT channel_hype_train_progress_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  int64_t total {0};
  int64_t progress {0};
  int64_t goal {0};
  std::vector<hype_train_contribution> top_contributions;
  hype_train_contribution last_contribution;
  int64_t level {0};
  bool is_shared_train {false};
  std::string started_at;
  std::string expires_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_hype_train_end for
 * a "channel.hype_train.end" notification.
 */
struct TPP_EXPORT channel_hype_train_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  int64_t total {0};
  int64_t level {0};
  bool is_shared_train {false};
  std::vector<hype_train_contribution> top_contributions;
  std::string started_at;
  std::string ended_at;
  std::string cooldown_ends_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_shield_mode_begin
 * for a "channel.shield_mode.begin" notification.
 */
struct TPP_EXPORT channel_shield_mode_begin_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  std::string started_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_shield_mode_end for
 * a "channel.shield_mode.end" notification.
 */
struct TPP_EXPORT channel_shield_mode_end_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user moderator;
  std::string ended_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_shoutout_create for
 * a "channel.shoutout.create" notification.
 */
struct TPP_EXPORT channel_shoutout_create_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user to_broadcaster;
  user moderator;
  int64_t viewer_count {0};
  std::string started_at;
  std::string cooldown_ends_at;
  std::string target_cooldown_ends_at;
};

/**
 * @brief Payload delivered through conduit::on_channel_shoutout_receive for
 * a "channel.shoutout.receive" notification.
 */
struct TPP_EXPORT channel_shoutout_receive_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
  user from_broadcaster;
  int64_t viewer_count {0};
  std::string started_at;
};

/**
 * @brief Payload delivered through conduit::on_conduit_shard_disabled for
 * a "conduit.shard.disabled" notification.
 */
struct TPP_EXPORT conduit_shard_disabled_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string conduit_id;
  std::string shard_id;
  std::string status;
  eventsub_transport transport;
};

/**
 * @brief Payload delivered through conduit::on_drop_entitlement_grant for a
 * "drop.entitlement.grant" notification.
 */
struct TPP_EXPORT drop_entitlement_grant_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string organization_id;
  std::string category_id;
  std::string category_name;
  std::string campaign_id;
  user recipient;
  std::string entitlement_id;
  std::string benefit_id;
  std::string created_at;
};

/**
 * @brief Payload delivered through
 * conduit::on_extension_bits_transaction_create for an
 * "extension.bits_transaction.create" notification.
 */
struct TPP_EXPORT extension_bits_transaction_create_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  std::string extension_client_id;
  user broadcaster;
  user purchaser;
  std::string product_name;
  std::string product_sku;
  int64_t product_bits {0};
  bool product_in_development {false};
};

/**
 * @brief Payload delivered through conduit::on_stream_online for a
 * "stream.online" notification.
 */
struct TPP_EXPORT stream_online_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string id;
  user broadcaster;
  /**
   * @brief One of "live", "playlist", "watch_party", "premiere", "rerun".
   */
  std::string type;
  std::string started_at;
};

/**
 * @brief Payload delivered through conduit::on_stream_offline for a
 * "stream.offline" notification.
 */
struct TPP_EXPORT stream_offline_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user broadcaster;
};

/**
 * @brief Payload delivered through conduit::on_user_authorization_grant
 * for a "user.authorization.grant" notification.
 */
struct TPP_EXPORT user_authorization_grant_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string client_id;
  user authorized;
};

/**
 * @brief Payload delivered through conduit::on_user_authorization_revoke
 * for a "user.authorization.revoke" notification.
 */
struct TPP_EXPORT user_authorization_revoke_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  std::string client_id;
  user revoked;
};

/**
 * @brief Payload delivered through conduit::on_user_update for a
 * "user.update" notification.
 */
struct TPP_EXPORT user_update_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user updated;
  std::string email;
  bool email_verified {false};
  std::string description;
};

/**
 * @brief Payload delivered through conduit::on_user_whisper_message for a
 * "user.whisper.message" notification.
 */
struct TPP_EXPORT user_whisper_message_t : public event_dispatch_t {
  using event_dispatch_t::event_dispatch_t;
  using event_dispatch_t::operator=;

  user from_user;
  user to_user;
  std::string whisper_id;
  std::string text;
};

}// namespace tpp
