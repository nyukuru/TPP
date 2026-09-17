#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"
#include "tpp/json_interface.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief One contribution to a hype train - reused for both
 * channel_hype_train_*_t::top_contributions elements and
 * ::last_contribution.
 */
struct TPP_EXPORT hype_train_contribution : public json_interface<hype_train_contribution> {
  friend struct json_interface<hype_train_contribution>;

  user contributor;
  /**
   * @brief One of "bits", "subscription", "other".
   */
  std::string type;
  int64_t total {0};

 protected:
  hype_train_contribution &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
