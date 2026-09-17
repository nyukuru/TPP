#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "tpp/event_router.h"
#include "tpp/export.h"
#include "tpp/scope.h"

namespace tpp {

class conduit;
class http_server;
struct http_server_request;
class https_client;
class consumer;

/**
 * @brief Runs the local HTTP redirect listener for a conduit's Twitch
 * OIDC implicit grant flow and resolves completed logins into
 * tpp::consumer objects, parented by the given conduit. A standalone
 * object, started and stopped independently of the owning conduit.
 * Requires the owning conduit to already be running.
 */
class TPP_EXPORT auth_server {
  conduit *owner_;
  uint16_t redirect_port_;
  std::string redirect_uri_;
  std::string oauth_state_;

  std::unique_ptr<http_server> server_;
  std::unique_ptr<https_client> user_lookup_;

  void handle_auth_request(http_server_request *request);
  void resolve_authenticated_user(const std::string &access_token, const std::string &id_token, uint64_t expires_in, const std::string &scope);

 public:
  /**
   * @param owner owning conduit; must outlive this auth_server
   * @param redirect_port Local port to listen on for the OAuth redirect.
   * Must match the port used in the application's registered "OAuth
   * Redirect URLs"
   * @throw std::invalid_argument if redirect_port is 0
   */
  auth_server(conduit *owner, uint16_t redirect_port);

  ~auth_server();

  auth_server(const auth_server &) = delete;
  auth_server &operator=(const auth_server &) = delete;
  auth_server(auth_server &&) = delete;
  auth_server &operator=(auth_server &&) = delete;

  /**
   * @brief Starts listening for the OAuth redirect. Non-blocking - like
   * every other socket in T++, IO is driven by the owning conduit's
   * socket engine, so this returns immediately once the listener is
   * registered with it.
   * @note The owning conduit must already be running (its socketengine
   * must exist).
   * @throw std::runtime_error if the owning conduit is not running
   */
  void start();

  /**
   * @brief Stops listening and drops the redirect listener.
   */
  void stop();

  [[nodiscard]] bool is_running() const noexcept;

  /**
   * @brief Builds the "https://id.twitch.tv/oauth2/authorize" URL for the
   * Twitch OIDC implicit grant flow, filled in with the owning conduit's
   * client_id and this server's redirect_uri.
   * @param scopes scopes to request
   * @param claims optional raw OIDC "claims" JSON parameter
   * @return complete authentication URL to send the user to
   */
  std::string generate_auth_url(const scope &scopes, const std::string &claims = "");

  /**
   * @brief Fired once a user has completed the Twitch OIDC implicit
   * grant flow. The consumer delivered is freshly constructed and not
   * tracked by the owning conduit; it is destroyed once every handler
   * has returned unless a handler tracks it.
   * @see conduit::add_consumer
   */
  event_router_t<std::shared_ptr<consumer>> on_authenticate;
};

}// namespace tpp
