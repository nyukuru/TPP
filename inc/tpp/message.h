#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "tpp/emote.h"
#include "tpp/export.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief One fragment of a Twitch chat message's text, split out by type
 * (plain text, an emote, a cheermote, or a mention) - mirrors Twitch's own
 * message.fragments array. Only the fields relevant to fragment::type are
 * populated; the rest are left at their defaults.
 */
struct TPP_EXPORT chat_fragment {
  /**
   * @brief One of "text", "cheermote", "emote", "mention".
   */
  std::string type;
  std::string text;

  /* cheermote */
  std::string cheermote_prefix;
  int64_t cheermote_bits {0};
  int64_t cheermote_tier {0};

  /* emote */
  emote emote_data;

  /* mention */
  user mentioned_user;

  chat_fragment &fill_from_json(nlohmann::json &j);
};

/**
 * @brief A Twitch chat message, reused across every EventSub notification
 * that embeds one - channel.chat.message, automod.message.hold/update,
 * channel.chat.user_message_hold/update, channel.subscription.message -
 * mirroring DPP's own dpp::message, which dpp::message_create_t and
 * friends likewise wrap rather than duplicating its fields.
 */
struct TPP_EXPORT message {
  user broadcaster;
  user chatter;
  std::string id;
  std::string text;
  std::vector<chat_fragment> fragments;

  /**
   * @brief Fills every field above from the parent object of a
   * notification that embeds a chat message.
   * @param j the notification's payload.event (or another object shaped
   * like it) - broadcaster_user_*, "message_id", and a nested "message"
   * object with "text"/"fragments" are read from here
   * @param chatter_prefix key prefix for the message author - most
   * events key it "chatter_user" (chatter_user_id/login/name, the
   * default); automod and the chat.user_message_* events key it plain
   * "user" (user_id/login/name) instead
   */
  message &fill_from_json(nlohmann::json &j, const std::string &chatter_prefix = "chatter_user");
};

}// namespace tpp
