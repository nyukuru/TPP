#pragma once

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
};

}// namespace tpp
