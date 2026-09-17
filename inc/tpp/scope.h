#pragma once

#include <bitset>
#include <cstddef>
#include <string>
#include <type_traits>

#include "tpp/export.h"

namespace tpp {

/**
 * @brief A set of tpp::scope::scopes. Serializes to the space-delimited
 * scope list Twitch expects in the "scope" query parameter of the
 * authorization URL, e.g. "chat:read chat:edit openid".
 */
class TPP_EXPORT scope {
 public:
  /**
   * @brief Twitch OAuth scopes. Names map to Twitch's scope strings,
   * colons replaced with underscores, e.g. s_channel_manage_polls is
   * "channel:manage:polls". Combine values with `|` or pass several to
   * tpp::scope's constructor.
   */
  enum scopes : std::size_t {
    /**
     * @brief View analytics data for the user's owned Twitch Extensions.
     */
    s_analytics_read_extensions,

    /**
     * @brief View analytics data for the user's games.
     */
    s_analytics_read_games,

    /**
     * @brief View Bits information for a channel.
     */
    s_bits_read,

    /**
     * @brief Join a channel's chatroom as a bot user and perform
     * chat-related actions.
     */
    s_channel_bot,

    /**
     * @brief Run commercials on a channel.
     */
    s_channel_edit_commercial,

    /**
     * @brief Manage a channel's ad schedule.
     */
    s_channel_manage_ads,

    /**
     * @brief Manage a channel's stream title, category, and other broadcast
     * settings.
     */
    s_channel_manage_broadcast,

    /**
     * @brief Manage Clips for a channel.
     */
    s_channel_manage_clips,

    /**
     * @brief Manage a channel's active Extensions.
     */
    s_channel_manage_extensions,

    /**
     * @brief Manage Guest Star for the user's channel.
     */
    s_channel_manage_guest_star,

    /**
     * @brief Add or remove the moderator role on the user's channel.
     */
    s_channel_manage_moderators,

    /**
     * @brief Manage a channel's polls.
     */
    s_channel_manage_polls,

    /**
     * @brief Manage a channel's Predictions.
     */
    s_channel_manage_predictions,

    /**
     * @brief Start or cancel a raid on another channel.
     */
    s_channel_manage_raids,

    /**
     * @brief Manage a channel's Channel Points custom rewards and
     * redemptions.
     */
    s_channel_manage_redemptions,

    /**
     * @brief Manage a channel's stream schedule.
     */
    s_channel_manage_schedule,

    /**
     * @brief Manage a channel's videos, including deleting them.
     */
    s_channel_manage_videos,

    /**
     * @brief Add or remove the VIP role on the user's channel.
     */
    s_channel_manage_vips,

    /**
     * @brief Perform moderation actions in a channel.
     */
    s_channel_moderate,

    /**
     * @brief View a channel's ad schedule and details.
     */
    s_channel_read_ads,

    /**
     * @brief View a channel's charity campaign and donations.
     */
    s_channel_read_charity,

    /**
     * @brief View the list of users with editor permissions on a channel.
     */
    s_channel_read_editors,

    /**
     * @brief View a channel's Creator Goals.
     */
    s_channel_read_goals,

    /**
     * @brief View Guest Star details for the user's channel.
     */
    s_channel_read_guest_star,

    /**
     * @brief View Hype Train information for a channel.
     */
    s_channel_read_hype_train,

    /**
     * @brief View a channel's polls.
     */
    s_channel_read_polls,

    /**
     * @brief View a channel's Predictions.
     */
    s_channel_read_predictions,

    /**
     * @brief View a channel's Channel Points custom rewards and
     * redemptions.
     */
    s_channel_read_redemptions,

    /**
     * @brief View a channel's stream key.
     */
    s_channel_read_stream_key,

    /**
     * @brief View the list of users subscribed to a channel.
     */
    s_channel_read_subscriptions,

    /**
     * @brief View the list of VIPs on the user's channel.
     */
    s_channel_read_vips,

    /**
     * @brief Send chat messages to a chatroom over the (legacy) IRC
     * connection.
     */
    s_chat_edit,

    /**
     * @brief View chat messages in a chatroom over the (legacy) IRC
     * connection.
     */
    s_chat_read,

    /**
     * @brief Create and edit Clips for a channel.
     */
    s_clips_edit,

    /**
     * @brief Manage Clips for a channel as an editor.
     */
    s_editor_manage_clips,

    /**
     * @brief View a channel's moderation data, including bans, timeouts, and
     * AutoMod settings.
     */
    s_moderation_read,

    /**
     * @brief Send announcements in channels the user moderates.
     */
    s_moderator_manage_announcements,

    /**
     * @brief Manage messages held for review by AutoMod in channels the
     * user moderates.
     */
    s_moderator_manage_automod,

    /**
     * @brief Manage a broadcaster's AutoMod settings.
     */
    s_moderator_manage_automod_settings,

    /**
     * @brief Ban and unban users in channels the user moderates.
     */
    s_moderator_manage_banned_users,

    /**
     * @brief Manage a broadcaster's list of blocked terms.
     */
    s_moderator_manage_blocked_terms,

    /**
     * @brief Delete chat messages in channels the user moderates.
     */
    s_moderator_manage_chat_messages,

    /**
     * @brief Manage a broadcaster's chat room settings.
     */
    s_moderator_manage_chat_settings,

    /**
     * @brief Manage Guest Star for channels the user moderates.
     */
    s_moderator_manage_guest_star,

    /**
     * @brief Manage a broadcaster's Shield Mode status.
     */
    s_moderator_manage_shield_mode,

    /**
     * @brief Send Shoutouts on behalf of channels the user moderates.
     */
    s_moderator_manage_shoutouts,

    /**
     * @brief Manage suspicious user status in channels the user moderates.
     */
    s_moderator_manage_suspicious_users,

    /**
     * @brief Resolve unban requests in channels the user moderates.
     */
    s_moderator_manage_unban_requests,

    /**
     * @brief Warn users in channels the user moderates.
     */
    s_moderator_manage_warnings,

    /**
     * @brief View a broadcaster's AutoMod settings.
     */
    s_moderator_read_automod_settings,

    /**
     * @brief View the list of banned/timed-out users in channels the user
     * moderates.
     */
    s_moderator_read_banned_users,

    /**
     * @brief View a broadcaster's list of blocked terms.
     */
    s_moderator_read_blocked_terms,

    /**
     * @brief View deleted chat messages in channels the user moderates.
     */
    s_moderator_read_chat_messages,

    /**
     * @brief View a broadcaster's chat room settings.
     */
    s_moderator_read_chat_settings,

    /**
     * @brief View the list of users in a channel's chat room.
     */
    s_moderator_read_chatters,

    /**
     * @brief View the followers of a broadcaster.
     */
    s_moderator_read_followers,

    /**
     * @brief View Guest Star details for channels the user moderates.
     */
    s_moderator_read_guest_star,

    /**
     * @brief View the list of moderators in channels the user moderates.
     */
    s_moderator_read_moderators,

    /**
     * @brief View a broadcaster's Shield Mode status.
     */
    s_moderator_read_shield_mode,

    /**
     * @brief View the Shoutouts given and received by a broadcaster.
     */
    s_moderator_read_shoutouts,

    /**
     * @brief View suspicious user activity in channels the user moderates.
     */
    s_moderator_read_suspicious_users,

    /**
     * @brief View unban requests in channels the user moderates.
     */
    s_moderator_read_unban_requests,

    /**
     * @brief View the list of VIPs in channels the user moderates.
     */
    s_moderator_read_vips,

    /**
     * @brief View warnings given to users in channels the user moderates.
     */
    s_moderator_read_warnings,

    /**
     * @brief Join a channel's chatroom as this user, appearing as a bot.
     */
    s_user_bot,

    /**
     * @brief Manage a user's profile information.
     */
    s_user_edit,

    /**
     * @brief View and edit a user's broadcasting configuration, including
     * Extension configuration.
     */
    s_user_edit_broadcast,

    /**
     * @brief Manage the block list of a user.
     */
    s_user_manage_blocked_users,

    /**
     * @brief Update the color used for the user's name in chat.
     */
    s_user_manage_chat_color,

    /**
     * @brief Receive and send whispers on the user's behalf.
     */
    s_user_manage_whispers,

    /**
     * @brief View the block list of a user.
     */
    s_user_read_blocked_users,

    /**
     * @brief View a user's broadcasting configuration, including Extension
     * configuration.
     */
    s_user_read_broadcast,

    /**
     * @brief Receive chat messages and notifications for a user via
     * EventSub.
     */
    s_user_read_chat,

    /**
     * @brief View a user's email address.
     */
    s_user_read_email,

    /**
     * @brief View emotes available to a user.
     */
    s_user_read_emotes,

    /**
     * @brief View the list of channels a user follows.
     */
    s_user_read_follows,

    /**
     * @brief View the list of channels a user has moderator privileges in.
     */
    s_user_read_moderated_channels,

    /**
     * @brief View whether a user is subscribed to specific channels.
     */
    s_user_read_subscriptions,

    /**
     * @brief Receive whispers sent to a user.
     */
    s_user_read_whispers,

    /**
     * @brief Send chat messages to a chatroom via EventSub/Helix, without an
     * IRC connection.
     */
    s_user_write_chat,

    /**
     * @brief Receive whisper messages for a user over the (legacy) PubSub
     * connection.
     */
    s_whispers_read,

    /**
     * @brief Requests an OIDC ID token alongside the access token.
     * conduit::on_authenticate receives a non-empty id_token only when
     * this scope is set.
     */
    s_openid,

    /**
     * @brief Not a real scope. The number of scopes above.
     */
    s_count,
  };

 protected:
  /**
   * @brief The set scopes.
   */
  std::bitset<s_count> value {};

 public:
  /**
   * @brief Constructs an empty scope set.
   */
  scope() = default;

  /**
   * @brief Constructs a scope set from one or more scopes.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param first a scope to set
   * @param rest further scopes to set
   */
  template<typename U, typename... T>
  explicit scope(U first, T... rest) noexcept {
    value.set(first);
    (value.set(rest), ...);
  }

  /**
   * @brief Checks whether all of the given scopes are set.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param values the scopes to check for
   * @return true if every given scope is set
   */
  template<typename... T>
  [[nodiscard]] bool has(T... values) const noexcept {
    return (value.test(values) && ...);
  }

  /**
   * @brief Checks whether any of the given scopes are set.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param values the scopes to check for
   * @return true if at least one given scope is set
   */
  template<typename... T>
  [[nodiscard]] bool has_any(T... values) const noexcept {
    return (value.test(values) || ...);
  }

  /**
   * @brief Sets one or more scopes.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param values the scopes to set
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), scope &> add(T... values) noexcept {
    (value.set(values), ...);
    return *this;
  }

  /**
   * @brief Clears every scope, then sets the given scopes.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param values the scopes to set
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), scope &> set(T... values) noexcept {
    value.reset();
    (value.set(values), ...);
    return *this;
  }

  /**
   * @brief Clears one or more scopes.
   * @tparam T one or more std::size_t-convertible scope positions
   * @param values the scopes to clear
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), scope &> remove(T... values) noexcept {
    (value.reset(values), ...);
    return *this;
  }

  /**
   * @brief Serializes the scope set to the space-delimited string Twitch
   * expects, e.g. "chat:read chat:edit openid".
   * @return the serialized scope list, or an empty string if no scopes
   * are set
   */
  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Union of two scope sets.
   * @return a scope set containing every scope in lhs or rhs
   */
  friend inline scope operator|(const scope &lhs, const scope &rhs) noexcept {
    scope result;
    result.value = lhs.value | rhs.value;
    return result;
  }

  /**
   * @brief Adds every scope in rhs to lhs.
   * @return reference to lhs
   */
  friend inline scope &operator|=(scope &lhs, const scope &rhs) noexcept {
    lhs.value |= rhs.value;
    return lhs;
  }

  /**
   * @brief Intersection of two scope sets.
   * @return a scope set containing only the scopes present in both lhs
   * and rhs
   */
  friend inline scope operator&(const scope &lhs, const scope &rhs) noexcept {
    scope result;
    result.value = lhs.value & rhs.value;
    return result;
  }

  /**
   * @brief Clears every scope in lhs that is not also set in rhs.
   * @return reference to lhs
   */
  friend inline scope &operator&=(scope &lhs, const scope &rhs) noexcept {
    lhs.value &= rhs.value;
    return lhs;
  }
};

/**
 * @brief Union of two scopes.
 * @return a scope set containing lhs and rhs
 */
inline scope operator|(scope::scopes lhs, scope::scopes rhs) noexcept {
  return scope(lhs, rhs);
}

/**
 * @brief Adds a scope to a scope set.
 * @return a scope set containing lhs and rhs
 */
inline scope operator|(const scope &lhs, scope::scopes rhs) noexcept {
  return lhs | scope(rhs);
}

/**
 * @brief Adds a scope to a scope set.
 * @return a scope set containing lhs and rhs
 */
inline scope operator|(scope::scopes lhs, const scope &rhs) noexcept {
  return scope(lhs) | rhs;
}

/**
 * @brief Adds a scope to a scope set.
 * @return reference to lhs
 */
inline scope &operator|=(scope &lhs, scope::scopes rhs) noexcept {
  return lhs |= scope(rhs);
}

/**
 * @brief Intersection of a scope set with a single scope.
 * @return a scope set containing rhs if lhs has it set, otherwise empty
 */
inline scope operator&(const scope &lhs, scope::scopes rhs) noexcept {
  return lhs & scope(rhs);
}

/**
 * @brief Intersection of a scope set with a single scope.
 * @return a scope set containing lhs if rhs has it set, otherwise empty
 */
inline scope operator&(scope::scopes lhs, const scope &rhs) noexcept {
  return scope(lhs) & rhs;
}

/**
 * @brief Intersection of two scopes.
 * @return a scope set containing lhs if lhs equals rhs, otherwise empty
 */
inline scope operator&(scope::scopes lhs, scope::scopes rhs) noexcept {
  return scope(lhs) & scope(rhs);
}

/**
 * @brief Clears lhs's scope unless it equals rhs.
 * @return reference to lhs
 */
inline scope &operator&=(scope &lhs, scope::scopes rhs) noexcept {
  return lhs &= scope(rhs);
}

}// namespace tpp
