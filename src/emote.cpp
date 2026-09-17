#include <tpp/emote.h>
#include <tpp/event.h>

namespace tpp {

emote &emote::fill_from_json_impl(nlohmann::json *j) {
  id = string_not_null(j, "id");
  emote_set_id = string_not_null(j, "emote_set_id");
  owner_id = string_not_null(j, "owner_id");
  for_each_json(j, "format", [this](nlohmann::json *fmt) {
    if (fmt->is_string()) {
      format.push_back(fmt->get<std::string>());
    }
  });
  return *this;
}

}// namespace tpp
