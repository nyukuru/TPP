#include <tpp/automod.h>
#include <tpp/event.h>

namespace tpp {

automod_boundary &automod_boundary::fill_from_json_impl(nlohmann::json *j) {
  start_pos = int64_not_null(j, "start_pos");
  end_pos = int64_not_null(j, "end_pos");
  return *this;
}

automod_check_result &automod_check_result::fill_from_json_impl(nlohmann::json *j) {
  category = string_not_null(j, "category");
  level = int64_not_null(j, "level");
  for_each_json(j, "boundaries", [this](nlohmann::json *elem) {
    automod_boundary b;
    b.fill_from_json(elem);
    boundaries.push_back(b);
  });
  return *this;
}

blocked_term_match &blocked_term_match::fill_from_json_impl(nlohmann::json *j) {
  term_id = string_not_null(j, "term_id");
  boundary.fill_from_json(&(*j)["boundary"]);
  owner_broadcaster.fill_from_json(*j, "owner_broadcaster_user");
  return *this;
}

blocked_term_check_result &blocked_term_check_result::fill_from_json_impl(nlohmann::json *j) {
  for_each_json(j, "terms_found", [this](nlohmann::json *elem) {
    blocked_term_match m;
    m.fill_from_json(elem);
    terms_found.push_back(m);
  });
  return *this;
}

}// namespace tpp
