#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "tpp/application.h"
#include "tpp/scope.h"
#include "tpp/session.h"

int main() {
  const char        *client_id     = std::getenv("TWITCH_CLIENT_ID");
  constexpr uint16_t redirect_port = 3000;

  try {
    tpp::application app(client_id, redirect_port);

    tpp::scope scopes =
        tpp::s_user_read_chat | tpp::s_user_write_chat | tpp::s_openid;

    std::string auth_url = app.generate_auth_url(scopes);
    std::cout << "Open this URL in your browser to authenticate:\n"
              << auth_url << std::endl;

    app.on_authenticate([](tpp::session &session) {
      std::cout << "Authenticated as " << session.get_login() << " (user id "
                << session.get_user_id() << ")" << std::endl;

      session.on_chat_message([&session](const tpp::user   &broadcaster,
                                         const tpp::user   &chatter,
                                         const std::string &message) {
        std::cout << "#" << broadcaster.login << " " << chatter.login << ": "
                  << message << std::endl;
        if (message == "ping") {
          session.send_message("pong");
        }
      });
    });

    std::cout << "Starting Twitch bot, waiting for authentication..."
              << std::endl;
    app.start(true);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
