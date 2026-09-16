#pragma once

#include <string>

#include "tpp/event.h"
#include "tpp/export.h"
#include "tpp/user.h"

namespace tpp {

/**
 * @brief Payload delivered through conduit::on_chat_message for a
 * "channel.chat.message" notification.
 */
struct TPP_EXPORT chat_message_t : public event_dispatch_t {
  user broadcaster;
  user chatter;
  std::string message;
};

}// namespace tpp
