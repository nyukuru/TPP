#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "tpp/export.h"
#include "tpp/json_interface.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief One of a Twitch prediction's top predictors - an element of
 * prediction_outcome::top_predictors.
 */
struct TPP_EXPORT prediction_top_predictor : public json_interface<prediction_top_predictor> {
  friend struct json_interface<prediction_top_predictor>;

  user predictor;
  int64_t channel_points_won {0};
  int64_t channel_points_used {0};

 protected:
  prediction_top_predictor &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief One outcome of a Twitch prediction.
 */
struct TPP_EXPORT prediction_outcome : public json_interface<prediction_outcome> {
  friend struct json_interface<prediction_outcome>;

  std::string id;
  std::string title;
  std::string color;
  int64_t users {0};
  int64_t channel_points {0};
  std::vector<prediction_top_predictor> top_predictors;

 protected:
  prediction_outcome &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
