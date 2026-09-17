#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"
#include "tpp/json_interface.h"

namespace tpp {

/**
 * @brief An on/off redemption limit with a numeric cap - shared by a
 * custom reward's max_per_stream and max_per_user_per_stream settings.
 */
struct TPP_EXPORT reward_limit_setting : public json_interface<reward_limit_setting> {
  friend struct json_interface<reward_limit_setting>;

  bool is_enabled {false};
  int64_t value {0};

 protected:
  reward_limit_setting &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief A custom reward's global_cooldown setting.
 */
struct TPP_EXPORT reward_cooldown_setting : public json_interface<reward_cooldown_setting> {
  friend struct json_interface<reward_cooldown_setting>;

  bool is_enabled {false};
  int64_t seconds {0};

 protected:
  reward_cooldown_setting &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief A custom reward's image or default_image setting.
 */
struct TPP_EXPORT reward_image : public json_interface<reward_image> {
  friend struct json_interface<reward_image>;

  std::string url_1x;
  std::string url_2x;
  std::string url_4x;

 protected:
  reward_image &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief A custom reward's summary as embedded in a redemption
 * notification - the "reward" object of a
 * channel.channel_points_custom_reward_redemption.add/update
 * notification.
 */
struct TPP_EXPORT redemption_reward : public json_interface<redemption_reward> {
  friend struct json_interface<redemption_reward>;

  std::string id;
  std::string title;
  int64_t cost {0};
  std::string prompt;

 protected:
  redemption_reward &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
