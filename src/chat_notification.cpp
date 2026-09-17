#include <tpp/chat_notification.h>
#include <tpp/event.h>

namespace tpp {

chat_badge &chat_badge::fill_from_json_impl(nlohmann::json *j) {
  set_id = string_not_null(j, "set_id");
  id = string_not_null(j, "id");
  info = string_not_null(j, "info");
  return *this;
}

chat_notice_metadata &chat_notice_metadata::fill_from_json_impl(nlohmann::json *j) {
  sub_tier = string_not_null(j, "sub_tier");
  is_prime = bool_not_null(j, "is_prime");
  duration_months = int64_not_null(j, "duration_months");
  is_anonymous = bool_not_null(j, "is_anonymous");
  cumulative_months = int64_not_null(j, "cumulative_months");
  streak_months = int64_not_null(j, "streak_months");
  auto streak_it = j->find("streak_months");
  streak_months_is_null = streak_it == j->end() || streak_it->is_null();
  is_gift = bool_not_null(j, "is_gift");
  gifter.fill_from_json(*j, "gifter_user");
  cumulative_total = int64_not_null(j, "cumulative_total");
  auto cum_it = j->find("cumulative_total");
  cumulative_total_is_null = cum_it == j->end() || cum_it->is_null();
  recipient.fill_from_json(*j, "recipient_user");
  community_gift_id = string_not_null(j, "community_gift_id");
  total = int64_not_null(j, "total");
  raider.fill_from_json(*j, "user");
  viewer_count = int64_not_null(j, "viewer_count");
  color = string_not_null(j, "color");
  bits_tier = int64_not_null(j, "tier");
  charity_name = string_not_null(j, "charity_name");
  nlohmann::json &donation_amount = (*j)["amount"];
  charity_amount_value = int64_not_null(&donation_amount, "value");
  charity_amount_decimal_places = int64_not_null(&donation_amount, "decimal_places");
  charity_amount_currency = string_not_null(&donation_amount, "currency");
  return *this;
}

}// namespace tpp
