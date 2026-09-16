#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "tpp/enums.h"
#include "tpp/event_router.h"
#include "tpp/export.h"
#include "tpp/intents.h"
#include "tpp/scope.h"
#include "tpp/socketengine.h"
#include "tpp/timer.h"

namespace tpp {

class http_server;
struct http_server_request;
class https_client;
class session;

/**
 * @brief Fired once a user has completed the Twitch OIDC implicit grant
 * flow and their tpp::session has been created. Register hooks (e.g.
 * session::on_chat_message) on the session from here.
 */
using authenticate_event = std::function<void(session &)>;

/**
 * @brief Fired with the result of application::get_app_access_token().
 */
using app_token_event = std::function<void(
    bool success, const std::string &access_token, uint64_t expires_in)>;

/**
 * @brief Fired with the result of application::refresh_user_token().
 */
using refresh_token_event =
    std::function<void(bool success, const std::string &access_token,
                       const std::string &refresh_token, uint64_t expires_in)>;

/**
 * @brief The top level object of a T++ program. Represents one registered
 * Twitch application (a client_id, optionally paired with its
 * client_secret), owns the shared IO event loop, and runs the Twitch OIDC
 * implicit grant flow to authenticate users.
 *
 * Each user who authenticates gets their own tpp::session, parented by
 * this application.
 */
class TPP_EXPORT application {
 private:
  std::string client_id_;
  std::string client_secret_;
  std::string redirect_uri_;
  uint16_t    redirect_port_ {0};
  std::string oauth_state_;

  intent                       intents_;
  std::atomic<bool>            running_ {false};
  std::unique_ptr<std::thread> worker_thread_;

  authenticate_event on_authenticate_ {};

  std::unique_ptr<http_server>  auth_server_;
  std::unique_ptr<https_client> user_lookup_;

  mutable std::mutex                                        sessions_mutex_;
  std::unordered_map<std::string, std::shared_ptr<session>> sessions_;

  /* Timer subsystem state, see tpp/timer.h */
  timer_next_t     next_timer;
  timers_deleted_t deleted_timers;
  std::mutex       timer_guard;

  void worker_loop();
  void handle_auth_request(http_server_request *request);
  void resolve_authenticated_user(const std::string &access_token,
                                  const std::string &id_token,
                                  uint64_t           expires_in,
                                  const std::string &scope);

  std::mutex                         deferred_mutex_;
  std::vector<std::function<void()>> deferred_;

  void run_deferred();

  std::list<std::unique_ptr<https_client>> pending_app_requests_;

 public:
  /**
   * @brief Socket engine driving all IO for this application.
   * @note stop() resets this to null. Any ssl_connection-derived object
   * you constructed directly against this application, other than one
   * owned by a session, must be destroyed before calling stop().
   */
  std::unique_ptr<socket_engine_base> socketengine;

  /**
   * @brief Fired whenever a socket managed by the socket engine is closed.
   */
  event_router_t<socket_close_t> on_socket_close;

  /**
   * @brief Queues a function to run on the next iteration of the IO event
   * loop.
   */
  void defer(std::function<void()> fn);

  /**
   * @param client_id Twitch application client ID
   * @param redirect_port Local port to listen on for the OAuth redirect.
   * Must match the port used in the application's registered "OAuth
   * Redirect URLs", e.g. http://localhost:3000
   * @param client_secret Optional. Required by get_app_access_token() and
   * refresh_user_token().
   * @param intent_flags EventSub subscription categories this application
   * intends to use. Informational only; sessions do not subscribe to
   * anything automatically - call session::subscribe() explicitly.
   */
  explicit application(const std::string &client_id, uint16_t redirect_port,
                       const std::string &client_secret = "",
                       intent intent_flags = intent(i_chat_messages));

  ~application();

  application(const application &)            = delete;
  application &operator=(const application &) = delete;
  application(application &&)                 = delete;
  application &operator=(application &&)      = delete;

  void start(bool wait = true);

  void stop();

  [[nodiscard]] bool is_running() const noexcept;

  [[nodiscard]] const std::string &get_client_id() const noexcept;

  [[nodiscard]] intent get_intents() const noexcept;

  /**
   * @brief Builds the "https://id.twitch.tv/oauth2/authorize" URL for the
   * Twitch OIDC implicit grant flow, filled in with this application's
   * client_id and redirect_uri.
   * @param scopes scopes to request
   * @param claims optional raw OIDC "claims" JSON parameter
   * @return complete authentication URL to send the user to
   */
  std::string generate_auth_url(const scope       &scopes,
                                const std::string &claims = "");

  /**
   * @brief Sets the callback fired once a user has completed the Twitch
   * OIDC implicit grant flow and their tpp::session has been created.
   */
  void on_authenticate(authenticate_event callback);

  /**
   * @brief Creates (or replaces) a session directly from an
   * already-obtained access token, without going through the local OAuth
   * redirect server.
   * @note Requires start() to have been called first.
   * @param user_id Twitch numeric user ID of the authenticated user
   * @param login Twitch login name of the authenticated user
   * @param access_token user access token
   * @param id_token optional OIDC ID token
   * @param expires_in optional, seconds until access_token expires
   * @param refresh_token optional refresh token. If given, the session
   * rotates access_token on its own shortly before expires_in runs out
   * @return the newly created session
   */
  std::shared_ptr<session> create_session(
      const std::string &user_id, const std::string &login,
      const std::string &access_token, const std::string &id_token = "",
      uint64_t expires_in = 0, const std::string &refresh_token = "");

  /**
   * @brief Looks up a previously created session by Twitch user ID.
   * @return the session, or nullptr if no such session exists
   */
  [[nodiscard]] std::shared_ptr<session> get_session(
      const std::string &user_id) const;

  /**
   * @brief Every session currently managed by this application.
   */
  [[nodiscard]] std::vector<std::shared_ptr<session>> get_sessions() const;

  /**
   * @brief Removes a session, closing its EventSub connection.
   */
  void remove_session(const std::string &user_id);

  /**
   * @brief Obtains an app access token via the OAuth2 Client Credentials
   * Grant. Requires client_secret. An app access token is not tied to any
   * user and cannot be used for user-scoped EventSub subscriptions, but
   * works with app-scoped Helix endpoints and client credential
   * validation.
   * @param callback called with the result; success is false if the
   * request failed or client_secret was not set
   */
  void get_app_access_token(app_token_event callback);

  /**
   * @brief Exchanges a refresh token for a new access token via the
   * OAuth2 Refresh Token Grant. Requires client_secret.
   * @param refresh_token the refresh token to redeem
   * @param callback called with the result; on success, a new
   * refresh_token may be included, replacing the old one
   */
  void refresh_user_token(const std::string  &refresh_token,
                          refresh_token_event callback);

  /**
   * @brief Logs a message. The default implementation writes to stderr.
   */
  void log(loglevel severity, const std::string &message) const;

  /**
   * @brief Starts a repeating timer.
   * @param on_tick called every `frequency` seconds
   * @param frequency seconds between ticks
   * @param on_stop optional, called once when the timer is stopped
   * @return timer handle, usable with stop_timer()
   */
  timer start_timer(timer_callback_t on_tick, uint64_t frequency,
                    timer_callback_t on_stop = {});

  /**
   * @brief Stops a previously started timer.
   * @param t timer handle from start_timer()
   * @return true always
   */
  bool stop_timer(timer t);

  /**
   * @brief Fires any due timers. Called from the socket engine roughly
   * once a second.
   */
  void tick_timers();
};

}// namespace tpp
