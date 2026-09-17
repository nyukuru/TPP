#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"

namespace tpp {

/**
 * @brief A Twitch user identity.
 */
struct TPP_EXPORT user {
  /**
   * @brief Twitch numeric user ID.
   */
  std::string id;

  /**
   * @brief Twitch login name.
   */
  std::string login;

  /**
   * @brief Twitch display name.
   */
  std::string name;

  [[nodiscard]] bool operator==(const user &other) const noexcept {
    return id == other.id && login == other.login && name == other.name;
  }

  [[nodiscard]] bool operator!=(const user &other) const noexcept {
    return !(*this == other);
  }

  /**
   * @brief Fills id/login/name from a "<prefix>_id"/"<prefix>_login"/
   * "<prefix>_name" triad, e.g. fill_from_json(j, "broadcaster_user") for
   * broadcaster_user_id/broadcaster_user_login/broadcaster_user_name.
   * Twitch flattens user identities into the parent object this way
   * rather than nesting them, unlike DPP's own user::fill_from_json().
   * @param j the parent object the prefixed keys live on
   * @param prefix key prefix, without the trailing "_id"/"_login"/"_name"
   */
  user &fill_from_json(const nlohmann::json &j, const std::string &prefix);
};

}// namespace tpp
