#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "tpp/export.h"
#include "tpp/json_interface.h"

namespace tpp {

/**
 * @brief The "emote" object of a chat_fragment whose type is "emote" -
 * describes one emote used within a Twitch chat message.
 */
struct TPP_EXPORT emote : public json_interface<emote> {
  friend struct json_interface<emote>;

  std::string id;
  std::string emote_set_id;
  std::string owner_id;
  std::vector<std::string> format;

 protected:
  emote &fill_from_json_impl(nlohmann::json *j);
};

}// namespace tpp
