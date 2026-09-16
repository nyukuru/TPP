#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "tpp/application.h"
#include "tpp/event.h"
#include "tpp/eventsub.h"
#include "tpp/scope.h"
#include "tpp/session.h"

int main() {
  const char        *client_id     = std::getenv("TWITCH_CLIENT_ID");
  constexpr uint16_t redirect_port = 3000;

  tpp::application app(client_id, redirect_port);

  tpp::scope scopes = tpp::scope::s_user_read_chat |
                      tpp::scope::s_user_write_chat | tpp::scope::s_openid;

  std::string auth_url = app.generate_auth_url(scopes);
  std::cout << "Open this URL in your browser to authenticate:\n"
            << auth_url << std::endl;

  app.on_authenticate([](tpp::session &sess) {
    std::cout << "Authenticated as " << sess.get_login() << " (user id "
              << sess.get_user_id() << ")" << std::endl;

    sess.on_chat_message([&sess](const tpp::chat_message_t &event) {
      std::cout << "#" << event.broadcaster.login << " " << event.chatter.login
                << ": " << event.message << std::endl;
      if (event.message == "ping") {
        sess.send_message("pong");
      }
    });

    sess.subscribe(tpp::event::channel_chat_message(sess.get_user_id()));
  });

  std::cout << "Starting Twitch bot, waiting for authentication..."
            << std::endl;
  app.start(true);

  return 0;
}
