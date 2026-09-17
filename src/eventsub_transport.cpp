#include <tpp/event.h>
#include <tpp/eventsub_transport.h>

namespace tpp {

eventsub_transport &eventsub_transport::fill_from_json_impl(nlohmann::json *j) {
  method = string_not_null(j, "method");
  callback = string_not_null(j, "callback");
  conduit_id = string_not_null(j, "conduit_id");
  return *this;
}

}// namespace tpp
