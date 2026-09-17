#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"
#include "tpp/json_interface.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief One chat badge (e.g. "moderator", "subscriber") worn by a
 * chatter - an element of channel_chat_notification_t::badges.
 */
struct TPP_EXPORT chat_badge : public json_interface<chat_badge> {
  friend struct json_interface<chat_badge>;

  std::string set_id;
  std::string id;
  std::string info;

 protected:
  chat_badge &fill_from_json_impl(nlohmann::json *j);
};

/**
 * @brief The sub-object named by channel_chat_notification_t::notice_type.
 * Every notice_type's fields are collapsed into this one struct; only
 * the fields relevant to the actual notice_type are populated.
 */
struct TPP_EXPORT chat_notice_metadata : public json_interface<chat_notice_metadata> {
  friend struct json_interface<chat_notice_metadata>;

  /* sub, resub, sub_gift, community_sub_gift, gift_paid_upgrade,
   * prime_paid_upgrade */
  std::string sub_tier;
  bool is_prime {false};
  int64_t duration_months {0};
  /* sub_gift, community_sub_gift, gift_paid_upgrade, pay_it_forward */
  bool is_anonymous {false};
  /* resub */
  int64_t cumulative_months {0};
  int64_t streak_months {0};
  bool streak_months_is_null {false};
  bool is_gift {false};
  user gifter;
  /* sub_gift, community_sub_gift */
  int64_t cumulative_total {0};
  bool cumulative_total_is_null {false};
  user recipient;
  std::string community_gift_id;
  /* community_sub_gift */
  int64_t total {0};
  /* raid */
  user raider;
  int64_t viewer_count {0};
  /* announcement */
  std::string color;
  /* bits_badge_tier */
  int64_t bits_tier {0};
  /* charity_donation */
  std::string charity_name;
  int64_t charity_amount_value {0};
  int64_t charity_amount_decimal_places {0};
  std::string charity_amount_currency;

 protected:
  chat_notice_metadata &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
