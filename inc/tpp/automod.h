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
 * @brief One boundary (start/end character offset) into a checked
 * message's text - shared by automod_check_result and
 * blocked_term_match.
 */
struct TPP_EXPORT automod_boundary : public json_interface<automod_boundary> {
  friend struct json_interface<automod_boundary>;

  int64_t start_pos {0};
  int64_t end_pos {0};

 protected:
  automod_boundary &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief AutoMod's own verdict on a held message - the "automod" object
 * of an automod.message.hold/update notification.
 */
struct TPP_EXPORT automod_check_result : public json_interface<automod_check_result> {
  friend struct json_interface<automod_check_result>;

  std::string category;
  int64_t level {0};
  std::vector<automod_boundary> boundaries;

 protected:
  automod_check_result &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief One blocked-term match found in a held message.
 */
struct TPP_EXPORT blocked_term_match : public json_interface<blocked_term_match> {
  friend struct json_interface<blocked_term_match>;

  std::string term_id;
  automod_boundary boundary;
  user owner_broadcaster;

 protected:
  blocked_term_match &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief The set of blocked-term matches found in a held message - the
 * "blocked_term" object of an automod.message.hold/update notification.
 */
struct TPP_EXPORT blocked_term_check_result : public json_interface<blocked_term_check_result> {
  friend struct json_interface<blocked_term_check_result>;

  std::vector<blocked_term_match> terms_found;

 protected:
  blocked_term_check_result &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
