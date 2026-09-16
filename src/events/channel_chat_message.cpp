#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>

namespace tpp::events {

namespace {

using json = nlohmann::json;

/**
 * @brief Reads a string field, returning an empty string if it is absent,
 * null, or not a string.
 */
std::string string_field(const json &j, const char *field) {
  auto it = j.find(field);
  if (it == j.end() || !it->is_string()) {
    return {};
  }
  return it->get<std::string>();
}

const json &object_field(const json &j, const char *field) {
  static const json empty = json::object();
  auto it = j.find(field);
  if (it == j.end() || !it->is_object()) {
    return empty;
  }
  return *it;
}

/**
 * @brief Parses "channel.chat.message" fields from a notification's
 * payload.event. Does not set chat_message_t::from or ::raw_event.
 */
chat_message_t parse_chat_message_event(const json &event) {
  chat_message_t fields;

  if (!event.is_object()) {
    return fields;
  }

  fields.broadcaster.id = string_field(event, "broadcaster_user_id");
  fields.broadcaster.login = string_field(event, "broadcaster_user_login");
  fields.broadcaster.name = string_field(event, "broadcaster_user_name");
  fields.chatter.id = string_field(event, "chatter_user_id");
  fields.chatter.login = string_field(event, "chatter_user_login");
  fields.chatter.name = string_field(event, "chatter_user_name");
  fields.message = string_field(object_field(event, "message"), "text");

  return fields;
}

}// namespace

void channel_chat_message::handle(consumer *s, nlohmann::json &j, const std::string &raw) const {
  chat_message_t event = parse_chat_message_event(j);
  event.from = s;
  event.raw_event = raw;
  s->get_conduit().on_chat_message.call(event);
}

}// namespace tpp::events
