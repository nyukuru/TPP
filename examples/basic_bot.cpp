#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "tpp/auth_server.h"
#include "tpp/conduit.h"
#include "tpp/consumer.h"
#include "tpp/dispatcher.h"
#include "tpp/event.h"
#include "tpp/scope.h"

int main() {
  const char *client_id = std::getenv("TWITCH_CLIENT_ID");
  const char *client_secret = std::getenv("TWITCH_CLIENT_SECRET");
  constexpr uint16_t redirect_port = 3000;

  tpp::conduit app(client_id, client_secret ? client_secret : "");
  tpp::auth_server auth(&app, redirect_port);

  tpp::scope scopes = tpp::scope::s_user_read_chat | tpp::scope::s_user_write_chat | tpp::scope::s_openid;

  std::string auth_url = auth.generate_auth_url(scopes);
  std::cout << "Open this URL in your browser to authenticate:\n" << auth_url << std::endl;

  /* Fires for every subscribed consumer - filter by event.from to scope
   * a handler to one authenticated broadcaster. */
  app.on_chat_message([](const tpp::chat_message_t &event) {
    std::cout << "#" << event.broadcaster.login << " " << event.chatter.login << ": " << event.message << std::endl;
    if (event.message == "ping") {
      event.from->send_message("pong");
    }
  });

  auth.on_authenticate([&app](std::shared_ptr<tpp::consumer> user) {
    std::cout << "Authenticated as " << user->get_login() << " (user id " << user->get_user_id() << ")" << std::endl;

    app.add_consumer(user);
    user->subscribe(tpp::event::channel_chat_message(user->get_user_id()));
  });

  std::cout << "Starting Twitch bot, waiting for authentication..." << std::endl;
  app.start(false);
  auth.start();
  app.open_conduit(1, [](bool ok) {
    if (!ok) {
      std::cerr << "Failed to open EventSub conduit" << std::endl;
    }
  });
  app.join();

  return 0;
}
