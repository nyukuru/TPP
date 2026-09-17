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
 * @brief The sub-object named by channel_moderate_t::action (e.g.
 * j["ban"] when action is "ban"). Every action's fields are collapsed
 * into this one struct, mirroring chat_notice_metadata - only the fields
 * relevant to the actual action are populated.
 */
struct TPP_EXPORT channel_moderate_action_metadata : public json_interface<channel_moderate_action_metadata> {
  friend struct json_interface<channel_moderate_action_metadata>;

  /* followers */
  int64_t follow_duration_minutes {0};
  /* slow */
  int64_t wait_time_seconds {0};
  /* vip, unvip, mod, unmod, ban, unban, timeout, untimeout, raid,
   * unraid, delete, unban_request, warn - the user the action targets */
  user target;
  /* ban, timeout, warn */
  std::string reason;
  /* timeout */
  std::string expires_at;
  /* raid */
  int64_t viewer_count {0};
  /* delete */
  std::string message_id;
  std::string message_body;
  /* automod_terms */
  std::string terms_action;
  std::string terms_list;
  std::vector<std::string> terms;
  bool terms_from_automod {false};
  /* unban_request */
  bool is_approved {false};
  std::string moderator_message;
  /* warn */
  std::vector<std::string> chat_rules_cited;

 protected:
  channel_moderate_action_metadata &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
