#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

#include <string>

namespace tpp {

namespace {

using json = nlohmann::json;

/**
 * @brief Splits a "wss://host[:port]/path" URL into hostname, port, and
 * path.
 */
struct ws_url_parts {
  std::string hostname;
  std::string port {"443"};
  std::string path {"/"};
};

ws_url_parts parse_ws_url(const std::string &url) {
  ws_url_parts parts;
  std::string rest = url;

  if (rest.rfind("wss://", 0) == 0) {
    rest = rest.substr(6);
  } else if (rest.rfind("ws://", 0) == 0) {
    rest = rest.substr(5);
    parts.port = "80";
  }

  auto slash = rest.find('/');
  std::string authority = slash == std::string::npos ? rest : rest.substr(0, slash);
  parts.path = slash == std::string::npos ? "/" : rest.substr(slash);

  auto colon = authority.find(':');
  if (colon == std::string::npos) {
    parts.hostname = authority;
  } else {
    parts.hostname = authority.substr(0, colon);
    parts.port = authority.substr(colon + 1);
  }

  return parts;
}

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
 * @brief Parses a single EventSub WebSocket text frame.
 */
eventsub_message parse_eventsub_message(const std::string &raw) {
  eventsub_message msg;

  json root = json::parse(raw, nullptr, false);
  if (root.is_discarded() || !root.is_object()) {
    return msg;
  }

  const json &metadata = object_field(root, "metadata");
  const json &payload = object_field(root, "payload");

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
  msg.session_id = string_field(session, "id");
  msg.reconnect_url = string_field(session, "reconnect_url");

  msg.subscription_type = string_field(metadata, "subscription_type");

  const json &event = object_field(payload, "event");
  if (!event.empty()) {
    msg.event = event;
  } else {
    /* Revocation messages carry "subscription" instead of "event" */
    msg.event = object_field(payload, "subscription");
  }
  msg.event_json = msg.event.dump();

  return msg;
}

}// namespace

eventsub_client::eventsub_client(conduit *creator, const std::string &connect_url)
    : websocket_client(creator, connect_url.empty() ? EVENTSUB_HOST : parse_ws_url(connect_url).hostname,
                       connect_url.empty() ? "443" : parse_ws_url(connect_url).port, connect_url.empty() ? EVENTSUB_PATH : parse_ws_url(connect_url).path) {
}

bool eventsub_client::handle_frame(const std::string &buffer, ws_opcode opcode) {
  if (opcode != OP_TEXT) {
    return true;
  }

  eventsub_message msg = parse_eventsub_message(buffer);
  switch (msg.type) {
    case eventsub_message_type::session_welcome:
      on_welcome.call({msg.session_id});
      break;
    case eventsub_message_type::session_reconnect:
      on_reconnect.call({msg.reconnect_url});
      break;
    case eventsub_message_type::notification:
      route_notification(msg.subscription_type, msg.event, msg.event_json);
      on_notification.call({msg.subscription_type, msg.event, msg.event_json});
      break;
    case eventsub_message_type::session_keepalive:
    case eventsub_message_type::revocation:
    case eventsub_message_type::unknown:
    default:
      break;
  }

  return true;
}

void eventsub_client::route_notification(const std::string &subscription_type, nlohmann::json event, const std::string &raw) {
  std::string user_id = event.value("broadcaster_user_id", "");
  if (user_id.empty()) {
    user_id = event.value("user_id", "");
  }
  if (user_id.empty()) {
    return;
  }

  auto c = owner->get_consumer(user_id);
  if (!c) {
    return;
  }

  owner->enqueue_dispatch([c, subscription_type, event, raw]() mutable { events::handle_event(c.get(), subscription_type, event, raw); });
}

void eventsub_client::log(tpp::loglevel severity, const std::string &msg) const {
  if (owner != nullptr) {
    owner->log(severity, msg);
  }
}

}// namespace tpp
