#include <tpp/event.h>
#include <tpp/user.h>

namespace tpp {

user &user::fill_from_json(const nlohmann::json &j, const std::string &prefix) {
  id = string_not_null(&j, (prefix + "_id").c_str());
  login = string_not_null(&j, (prefix + "_login").c_str());
  name = string_not_null(&j, (prefix + "_name").c_str());
  return *this;
}

}// namespace tpp
