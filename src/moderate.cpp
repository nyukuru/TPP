#include <tpp/event.h>
#include <tpp/moderate.h>

namespace tpp {

channel_moderate_action_metadata &channel_moderate_action_metadata::fill_from_json_impl(nlohmann::json *j) {
  follow_duration_minutes = int64_not_null(j, "follow_duration_minutes");
  wait_time_seconds = int64_not_null(j, "wait_time_seconds");
  target.fill_from_json(*j, "user");
  reason = string_not_null(j, "reason");
  expires_at = string_not_null(j, "expires_at");
  viewer_count = int64_not_null(j, "viewer_count");
  message_id = string_not_null(j, "message_id");
  message_body = string_not_null(j, "message_body");
  terms_action = string_not_null(j, "action");
  terms_list = string_not_null(j, "list");
  for_each_json(j, "terms", [this](nlohmann::json *elem) {
    if (elem->is_string()) {
      terms.push_back(elem->get<std::string>());
    }
  });
  terms_from_automod = bool_not_null(j, "from_automod");
  is_approved = bool_not_null(j, "is_approved");
  moderator_message = string_not_null(j, "moderator_message");
  for_each_json(j, "chat_rules_cited", [this](nlohmann::json *elem) {
    if (elem->is_string()) {
      chat_rules_cited.push_back(elem->get<std::string>());
    }
  });
  return *this;
}

}// namespace tpp
