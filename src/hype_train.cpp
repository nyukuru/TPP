#include <tpp/event.h>
#include <tpp/hype_train.h>

namespace tpp {

hype_train_contribution &hype_train_contribution::fill_from_json_impl(nlohmann::json *j) {
  contributor.fill_from_json(*j, "user");
  type = string_not_null(j, "type");
  total = int64_not_null(j, "total");
  return *this;
}

}// namespace tpp
