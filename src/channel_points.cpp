#include <tpp/channel_points.h>
#include <tpp/event.h>

namespace tpp {

reward_limit_setting &reward_limit_setting::fill_from_json_impl(nlohmann::json *j) {
  is_enabled = bool_not_null(j, "is_enabled");
  value = int64_not_null(j, "value");
  return *this;
}

reward_cooldown_setting &reward_cooldown_setting::fill_from_json_impl(nlohmann::json *j) {
  is_enabled = bool_not_null(j, "is_enabled");
  seconds = int64_not_null(j, "seconds");
  return *this;
}

reward_image &reward_image::fill_from_json_impl(nlohmann::json *j) {
  url_1x = string_not_null(j, "url_1x");
  url_2x = string_not_null(j, "url_2x");
  url_4x = string_not_null(j, "url_4x");
  return *this;
}

redemption_reward &redemption_reward::fill_from_json_impl(nlohmann::json *j) {
  id = string_not_null(j, "id");
  title = string_not_null(j, "title");
  cost = int64_not_null(j, "cost");
  prompt = string_not_null(j, "prompt");
  return *this;
}

}// namespace tpp
