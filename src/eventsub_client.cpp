#include <tpp/application.h>
#include <tpp/eventsub_client.h>

#include <string>

namespace tpp {

namespace {

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
  std::string  rest = url;

  if (rest.rfind("wss://", 0) == 0) {
    rest = rest.substr(6);
  } else if (rest.rfind("ws://", 0) == 0) {
    rest       = rest.substr(5);
    parts.port = "80";
  }

  auto        slash = rest.find('/');
  std::string authority =
      slash == std::string::npos ? rest : rest.substr(0, slash);
  parts.path = slash == std::string::npos ? "/" : rest.substr(slash);

  auto colon = authority.find(':');
  if (colon == std::string::npos) {
    parts.hostname = authority;
  } else {
    parts.hostname = authority.substr(0, colon);
    parts.port     = authority.substr(colon + 1);
  }

  return parts;
}

}// namespace

eventsub_client::eventsub_client(application       *creator,
                                 const std::string &connect_url)
    : websocket_client(
          creator,
          connect_url.empty() ? EVENTSUB_HOST
                              : parse_ws_url(connect_url).hostname,
          connect_url.empty() ? "443" : parse_ws_url(connect_url).port,
          connect_url.empty() ? EVENTSUB_PATH
                              : parse_ws_url(connect_url).path) {
}

bool eventsub_client::handle_frame(const std::string &buffer,
                                   ws_opcode          opcode) {
  if (opcode != OP_TEXT) {
    return true;
  }

  eventsub_message msg = parse_eventsub_message(buffer);
  switch (msg.type) {
    case eventsub_message_type::session_welcome:
      if (on_welcome) {
        on_welcome(msg.session_id);
      }
      break;
    case eventsub_message_type::session_reconnect:
      if (on_reconnect) {
        on_reconnect(msg.reconnect_url);
      }
      break;
    case eventsub_message_type::notification:
      if (on_notification) {
        on_notification(msg.subscription_type, msg.event, msg.event_json);
      }
      break;
    case eventsub_message_type::session_keepalive:
    case eventsub_message_type::revocation:
    case eventsub_message_type::unknown:
    default:
      break;
  }

  return true;
}

void eventsub_client::log(tpp::loglevel      severity,
                          const std::string &msg) const {
  if (owner != nullptr) {
    owner->log(severity, msg);
  }
}

}// namespace tpp
