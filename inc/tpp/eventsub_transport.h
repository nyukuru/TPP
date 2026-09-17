#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "tpp/export.h"
#include "tpp/json_interface.h"

namespace tpp {

/**
 * @brief An EventSub transport descriptor - the "transport" object of a
 * conduit.shard.disabled notification.
 */
struct TPP_EXPORT eventsub_transport : public json_interface<eventsub_transport> {
  friend struct json_interface<eventsub_transport>;

  std::string method;
  std::string callback;
  std::string conduit_id;

 protected:
  eventsub_transport &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
