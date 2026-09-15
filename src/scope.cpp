#include <tpp/scope.h>

#include <array>
#include <string>
#include <utility>

namespace tpp {

namespace {

constexpr std::array<std::pair<scopes, const char *>, 81> scope_names {
    {
     {s_analytics_read_extensions, "analytics:read:extensions"},
     {s_analytics_read_games, "analytics:read:games"},
     {s_bits_read, "bits:read"},
     {s_channel_bot, "channel:bot"},
     {s_channel_edit_commercial, "channel:edit:commercial"},
     {s_channel_manage_ads, "channel:manage:ads"},
     {s_channel_manage_broadcast, "channel:manage:broadcast"},
     {s_channel_manage_clips, "channel:manage:clips"},
     {s_channel_manage_extensions, "channel:manage:extensions"},
     {s_channel_manage_guest_star, "channel:manage:guest_star"},
     {s_channel_manage_moderators, "channel:manage:moderators"},
     {s_channel_manage_polls, "channel:manage:polls"},
     {s_channel_manage_predictions, "channel:manage:predictions"},
     {s_channel_manage_raids, "channel:manage:raids"},
     {s_channel_manage_redemptions, "channel:manage:redemptions"},
     {s_channel_manage_schedule, "channel:manage:schedule"},
     {s_channel_manage_videos, "channel:manage:videos"},
     {s_channel_manage_vips, "channel:manage:vips"},
     {s_channel_moderate, "channel:moderate"},
     {s_channel_read_ads, "channel:read:ads"},
     {s_channel_read_charity, "channel:read:charity"},
     {s_channel_read_editors, "channel:read:editors"},
     {s_channel_read_goals, "channel:read:goals"},
     {s_channel_read_guest_star, "channel:read:guest_star"},
     {s_channel_read_hype_train, "channel:read:hype_train"},
     {s_channel_read_polls, "channel:read:polls"},
     {s_channel_read_predictions, "channel:read:predictions"},
     {s_channel_read_redemptions, "channel:read:redemptions"},
     {s_channel_read_stream_key, "channel:read:stream_key"},
     {s_channel_read_subscriptions, "channel:read:subscriptions"},
     {s_channel_read_vips, "channel:read:vips"},
     {s_chat_edit, "chat:edit"},
     {s_chat_read, "chat:read"},
     {s_clips_edit, "clips:edit"},
     {s_editor_manage_clips, "editor:manage:clips"},
     {s_moderation_read, "moderation:read"},
     {s_moderator_manage_announcements, "moderator:manage:announcements"},
     {s_moderator_manage_automod, "moderator:manage:automod"},
     {s_moderator_manage_automod_settings,
         "moderator:manage:automod_settings"},
     {s_moderator_manage_banned_users, "moderator:manage:banned_users"},
     {s_moderator_manage_blocked_terms, "moderator:manage:blocked_terms"},
     {s_moderator_manage_chat_messages, "moderator:manage:chat_messages"},
     {s_moderator_manage_chat_settings, "moderator:manage:chat_settings"},
     {s_moderator_manage_guest_star, "moderator:manage:guest_star"},
     {s_moderator_manage_shield_mode, "moderator:manage:shield_mode"},
     {s_moderator_manage_shoutouts, "moderator:manage:shoutouts"},
     {s_moderator_manage_suspicious_users,
         "moderator:manage:suspicious_users"},
     {s_moderator_manage_unban_requests, "moderator:manage:unban_requests"},
     {s_moderator_manage_warnings, "moderator:manage:warnings"},
     {s_moderator_read_automod_settings, "moderator:read:automod_settings"},
     {s_moderator_read_banned_users, "moderator:read:banned_users"},
     {s_moderator_read_blocked_terms, "moderator:read:blocked_terms"},
     {s_moderator_read_chat_messages, "moderator:read:chat_messages"},
     {s_moderator_read_chat_settings, "moderator:read:chat_settings"},
     {s_moderator_read_chatters, "moderator:read:chatters"},
     {s_moderator_read_followers, "moderator:read:followers"},
     {s_moderator_read_guest_star, "moderator:read:guest_star"},
     {s_moderator_read_moderators, "moderator:read:moderators"},
     {s_moderator_read_shield_mode, "moderator:read:shield_mode"},
     {s_moderator_read_shoutouts, "moderator:read:shoutouts"},
     {s_moderator_read_suspicious_users, "moderator:read:suspicious_users"},
     {s_moderator_read_unban_requests, "moderator:read:unban_requests"},
     {s_moderator_read_vips, "moderator:read:vips"},
     {s_moderator_read_warnings, "moderator:read:warnings"},
     {s_user_bot, "user:bot"},
     {s_user_edit, "user:edit"},
     {s_user_edit_broadcast, "user:edit:broadcast"},
     {s_user_manage_blocked_users, "user:manage:blocked_users"},
     {s_user_manage_chat_color, "user:manage:chat_color"},
     {s_user_manage_whispers, "user:manage:whispers"},
     {s_user_read_blocked_users, "user:read:blocked_users"},
     {s_user_read_broadcast, "user:read:broadcast"},
     {s_user_read_chat, "user:read:chat"},
     {s_user_read_email, "user:read:email"},
     {s_user_read_emotes, "user:read:emotes"},
     {s_user_read_follows, "user:read:follows"},
     {s_user_read_moderated_channels, "user:read:moderated_channels"},
     {s_user_read_subscriptions, "user:read:subscriptions"},
     {s_user_read_whispers, "user:read:whispers"},
     {s_user_write_chat, "user:write:chat"},
     {s_whispers_read, "whispers:read"},
     }
};

}// namespace

std::string scope::to_string() const {
  std::string out;
  if (has(s_openid)) {
    out += "openid";
  }
  for (const auto &[bit, name] : scope_names) {
    if (has(bit)) {
      if (!out.empty()) {
        out += ' ';
      }
      out += name;
    }
  }
  return out;
}

}// namespace tpp
