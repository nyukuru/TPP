#include <tpp/scope.h>

#include <array>
#include <string>
#include <utility>

namespace tpp {

namespace {

constexpr std::array<std::pair<scope::scopes, const char *>, 81> scope_names {
    {
     {scope::s_analytics_read_extensions, "analytics:read:extensions"},
     {scope::s_analytics_read_games, "analytics:read:games"},
     {scope::s_bits_read, "bits:read"},
     {scope::s_channel_bot, "channel:bot"},
     {scope::s_channel_edit_commercial, "channel:edit:commercial"},
     {scope::s_channel_manage_ads, "channel:manage:ads"},
     {scope::s_channel_manage_broadcast, "channel:manage:broadcast"},
     {scope::s_channel_manage_clips, "channel:manage:clips"},
     {scope::s_channel_manage_extensions, "channel:manage:extensions"},
     {scope::s_channel_manage_guest_star, "channel:manage:guest_star"},
     {scope::s_channel_manage_moderators, "channel:manage:moderators"},
     {scope::s_channel_manage_polls, "channel:manage:polls"},
     {scope::s_channel_manage_predictions, "channel:manage:predictions"},
     {scope::s_channel_manage_raids, "channel:manage:raids"},
     {scope::s_channel_manage_redemptions, "channel:manage:redemptions"},
     {scope::s_channel_manage_schedule, "channel:manage:schedule"},
     {scope::s_channel_manage_videos, "channel:manage:videos"},
     {scope::s_channel_manage_vips, "channel:manage:vips"},
     {scope::s_channel_moderate, "channel:moderate"},
     {scope::s_channel_read_ads, "channel:read:ads"},
     {scope::s_channel_read_charity, "channel:read:charity"},
     {scope::s_channel_read_editors, "channel:read:editors"},
     {scope::s_channel_read_goals, "channel:read:goals"},
     {scope::s_channel_read_guest_star, "channel:read:guest_star"},
     {scope::s_channel_read_hype_train, "channel:read:hype_train"},
     {scope::s_channel_read_polls, "channel:read:polls"},
     {scope::s_channel_read_predictions, "channel:read:predictions"},
     {scope::s_channel_read_redemptions, "channel:read:redemptions"},
     {scope::s_channel_read_stream_key, "channel:read:stream_key"},
     {scope::s_channel_read_subscriptions, "channel:read:subscriptions"},
     {scope::s_channel_read_vips, "channel:read:vips"},
     {scope::s_chat_edit, "chat:edit"},
     {scope::s_chat_read, "chat:read"},
     {scope::s_clips_edit, "clips:edit"},
     {scope::s_editor_manage_clips, "editor:manage:clips"},
     {scope::s_moderation_read, "moderation:read"},
     {scope::s_moderator_manage_announcements,
         "moderator:manage:announcements"},
     {scope::s_moderator_manage_automod, "moderator:manage:automod"},
     {scope::s_moderator_manage_automod_settings,
         "moderator:manage:automod_settings"},
     {scope::s_moderator_manage_banned_users,
         "moderator:manage:banned_users"},
     {scope::s_moderator_manage_blocked_terms,
         "moderator:manage:blocked_terms"},
     {scope::s_moderator_manage_chat_messages,
         "moderator:manage:chat_messages"},
     {scope::s_moderator_manage_chat_settings,
         "moderator:manage:chat_settings"},
     {scope::s_moderator_manage_guest_star, "moderator:manage:guest_star"},
     {scope::s_moderator_manage_shield_mode, "moderator:manage:shield_mode"},
     {scope::s_moderator_manage_shoutouts, "moderator:manage:shoutouts"},
     {scope::s_moderator_manage_suspicious_users,
         "moderator:manage:suspicious_users"},
     {scope::s_moderator_manage_unban_requests,
         "moderator:manage:unban_requests"},
     {scope::s_moderator_manage_warnings, "moderator:manage:warnings"},
     {scope::s_moderator_read_automod_settings,
         "moderator:read:automod_settings"},
     {scope::s_moderator_read_banned_users, "moderator:read:banned_users"},
     {scope::s_moderator_read_blocked_terms, "moderator:read:blocked_terms"},
     {scope::s_moderator_read_chat_messages, "moderator:read:chat_messages"},
     {scope::s_moderator_read_chat_settings, "moderator:read:chat_settings"},
     {scope::s_moderator_read_chatters, "moderator:read:chatters"},
     {scope::s_moderator_read_followers, "moderator:read:followers"},
     {scope::s_moderator_read_guest_star, "moderator:read:guest_star"},
     {scope::s_moderator_read_moderators, "moderator:read:moderators"},
     {scope::s_moderator_read_shield_mode, "moderator:read:shield_mode"},
     {scope::s_moderator_read_shoutouts, "moderator:read:shoutouts"},
     {scope::s_moderator_read_suspicious_users,
         "moderator:read:suspicious_users"},
     {scope::s_moderator_read_unban_requests,
         "moderator:read:unban_requests"},
     {scope::s_moderator_read_vips, "moderator:read:vips"},
     {scope::s_moderator_read_warnings, "moderator:read:warnings"},
     {scope::s_user_bot, "user:bot"},
     {scope::s_user_edit, "user:edit"},
     {scope::s_user_edit_broadcast, "user:edit:broadcast"},
     {scope::s_user_manage_blocked_users, "user:manage:blocked_users"},
     {scope::s_user_manage_chat_color, "user:manage:chat_color"},
     {scope::s_user_manage_whispers, "user:manage:whispers"},
     {scope::s_user_read_blocked_users, "user:read:blocked_users"},
     {scope::s_user_read_broadcast, "user:read:broadcast"},
     {scope::s_user_read_chat, "user:read:chat"},
     {scope::s_user_read_email, "user:read:email"},
     {scope::s_user_read_emotes, "user:read:emotes"},
     {scope::s_user_read_follows, "user:read:follows"},
     {scope::s_user_read_moderated_channels, "user:read:moderated_channels"},
     {scope::s_user_read_subscriptions, "user:read:subscriptions"},
     {scope::s_user_read_whispers, "user:read:whispers"},
     {scope::s_user_write_chat, "user:write:chat"},
     {scope::s_whispers_read, "whispers:read"},
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
