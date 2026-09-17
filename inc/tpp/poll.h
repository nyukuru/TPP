#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"
#include "tpp/json_interface.h"

namespace tpp {

/**
 * @brief One choice in a channel points poll.
 */
struct TPP_EXPORT poll_choice : public json_interface<poll_choice> {
  friend struct json_interface<poll_choice>;

  std::string id;
  std::string title;
  int64_t bits_votes {0};
  int64_t channel_points_votes {0};
  int64_t votes {0};

 protected:
  poll_choice &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief A poll's bits_voting or channel_points_voting setting.
 */
struct TPP_EXPORT poll_voting : public json_interface<poll_voting> {
  friend struct json_interface<poll_voting>;

  bool is_enabled {false};
  int64_t amount_per_vote {0};

 protected:
  poll_voting &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
