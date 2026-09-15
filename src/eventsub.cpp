#include <tpp/eventsub.h>

#include <nlohmann/json.hpp>

namespace tpp {

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
  auto              it    = j.find(field);
  if (it == j.end() || !it->is_object()) {
    return empty;
  }
  return *it;
}

}// namespace

eventsub_message parse_eventsub_message(const std::string &raw) {
  eventsub_message msg;

  json root = json::parse(raw, nullptr, false);
  if (root.is_discarded() || !root.is_object()) {
    return msg;
  }

  const json &metadata = object_field(root, "metadata");
  const json &payload  = object_field(root, "payload");

  std::string message_type = string_field(metadata, "message_type");
  if (message_type == "session_welcome") {
    msg.type = eventsub_message_type::session_welcome;
  } else if (message_type == "session_keepalive") {
    msg.type = eventsub_message_type::session_keepalive;
  } else if (message_type == "session_reconnect") {
    msg.type = eventsub_message_type::session_reconnect;
  } else if (message_type == "notification") {
    msg.type = eventsub_message_type::notification;
  } else if (message_type == "revocation") {
    msg.type = eventsub_message_type::revocation;
  }

  const json &session = object_field(payload, "session");
  msg.session_id      = string_field(session, "id");
  msg.reconnect_url   = string_field(session, "reconnect_url");

  msg.subscription_type = string_field(metadata, "subscription_type");

  const json &event = object_field(payload, "event");
  if (!event.empty()) {
    msg.event_json = event.dump();
  } else {
    /* Revocation messages carry "subscription" instead of "event" */
    msg.event_json = object_field(payload, "subscription").dump();
  }

  return msg;
}

chat_message_event_fields parse_chat_message_event(
    const std::string &event_json) {
  chat_message_event_fields fields;

  json event = json::parse(event_json, nullptr, false);
  if (event.is_discarded() || !event.is_object()) {
    return fields;
  }

  fields.broadcaster.id    = string_field(event, "broadcaster_user_id");
  fields.broadcaster.login = string_field(event, "broadcaster_user_login");
  fields.broadcaster.name  = string_field(event, "broadcaster_user_name");
  fields.chatter.id        = string_field(event, "chatter_user_id");
  fields.chatter.login     = string_field(event, "chatter_user_login");
  fields.chatter.name      = string_field(event, "chatter_user_name");
  fields.message_text = string_field(object_field(event, "message"), "text");

  return fields;
}

}// namespace tpp
