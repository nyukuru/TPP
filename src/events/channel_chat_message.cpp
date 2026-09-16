#include <tpp/event.h>
#include <tpp/eventsub.h>
#include <tpp/session.h>

namespace tpp::events {

void channel_chat_message::handle(session *s, nlohmann::json &j,
                                  const std::string &raw) const {
  chat_message_t event = parse_chat_message_event(j);
  event.from           = s;
  event.raw_event      = raw;
  s->on_chat_message.call(event);
}

}// namespace tpp::events
