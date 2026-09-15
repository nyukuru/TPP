#include <internal/utility.h>
#include <openssl/rand.h>
#include <tpp/application.h>
#include <tpp/http_server.h>
#include <tpp/https_client.h>
#include <tpp/session.h>

#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>

namespace tpp {

namespace {

using json = nlohmann::json;

/**
 * @brief Generates a random hex nonce for use as an OAuth "state" or OIDC
 * "nonce" value.
 */
std::string random_hex_nonce() {
  unsigned char raw[16];
  RAND_bytes(raw, sizeof(raw));
  std::string out;
  out.reserve(sizeof(raw) * 2);
  static constexpr char hex[] = "0123456789abcdef";
  for (unsigned char byte : raw) {
    out.push_back(hex[(byte >> 4) & 0x0f]);
    out.push_back(hex[byte & 0x0f]);
  }
  return out;
}

/**
 * @brief Reads the OAuth tokens out of the URL fragment with JavaScript
 * and reloads the page with them turned into a query string.
 */
constexpr const char *fragment_forward_page =
    "<!doctype html><html><head><title>Signing in...</title></head><body>"
    "<script>"
    "var h = window.location.hash.substring(1);"
    "window.location.replace('/?' + h);"
    "</script>"
    "Completing sign-in&hellip;"
    "</body></html>";

constexpr const char *auth_success_page =
    "<!doctype html><html><head><title>Signed in</title></head><body>"
    "You are now signed in. You may close this window."
    "</body></html>";

constexpr const char *auth_failure_page =
    "<!doctype html><html><head><title>Sign-in failed</title></head><body>"
    "Sign-in failed, please close this window and try again."
    "</body></html>";

}// namespace

application::application(const std::string &client_id, uint16_t redirect_port,
                         const std::string &client_secret, intent intent_flags)
    : client_id_(client_id)
    , client_secret_(client_secret)
    , intents_(intent_flags) {
  if (client_id.empty()) {
    throw std::invalid_argument("Client ID cannot be empty");
  }
  if (redirect_port == 0) {
    throw std::invalid_argument("Redirect port cannot be zero");
  }
  redirect_port_ = redirect_port;
  redirect_uri_  = "http://localhost:" + std::to_string(redirect_port);
}

application::~application() {
  if (running_.load()) {
    stop();
  }
}

void application::start(bool wait) {
  if (running_.load()) {
    throw std::runtime_error("Application is already running");
  }

  socketengine = create_socket_engine(this);

  auth_server_ = std::make_unique<http_server>(
      this, "127.0.0.1", redirect_port_,
      [this](http_server_request *request) { handle_auth_request(request); });

  running_.store(true);
  worker_thread_ =
      std::make_unique<std::thread>(&application::worker_loop, this);

  if (wait && worker_thread_ && worker_thread_->joinable()) {
    worker_thread_->join();
  }
}

void application::stop() {
  if (!running_.load()) {
    return;
  }

  running_.store(false);

  if (worker_thread_ && worker_thread_->joinable()) {
    worker_thread_->join();
  }
  worker_thread_.reset();

  /* Drains work queued while stopping, before it can touch socketengine
   * after that is reset below. */
  run_deferred();

  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.clear();
  }
  user_lookup_.reset();
  auth_server_.reset();
  socketengine.reset();
}

bool application::is_running() const noexcept {
  return running_.load();
}

const std::string &application::get_client_id() const noexcept {
  return client_id_;
}

intent application::get_intents() const noexcept {
  return intents_;
}

void application::worker_loop() {
  while (running_.load()) {
    run_deferred();
    socketengine->process_events();
  }
}

void application::defer(std::function<void()> fn) {
  std::lock_guard<std::mutex> lock(deferred_mutex_);
  deferred_.push_back(std::move(fn));
}

void application::run_deferred() {
  std::vector<std::function<void()>> pending;
  {
    std::lock_guard<std::mutex> lock(deferred_mutex_);
    pending.swap(deferred_);
  }
  for (auto &fn : pending) {
    fn();
  }
}

void application::log(loglevel severity, const std::string &message) const {
  static constexpr const char *labels[] = {"TRACE", "DEBUG", "INFO",
                                           "WARN",  "ERROR", "CRITICAL"};
  const char                  *label =
      (severity >= 0 && severity <= ll_critical) ? labels[severity] : "?";
  std::cerr << "[" << label << "] " << message << std::endl;
}

std::string application::generate_auth_url(const scope       &scopes,
                                           const std::string &claims) {
  oauth_state_ = random_hex_nonce();

  std::string response_type = "token";
  std::string nonce;
  if (scopes.has(s_openid)) {
    response_type = "token+id_token";
    nonce         = random_hex_nonce();
  }

  std::string url =
      "https://id.twitch.tv/oauth2/authorize"
      "?response_type=" +
      response_type + "&client_id=" + utility::url_encode(client_id_) +
      "&redirect_uri=" + utility::url_encode(redirect_uri_) +
      "&scope=" + utility::url_encode(scopes.to_string()) +
      "&state=" + oauth_state_;

  if (!nonce.empty()) {
    url += "&nonce=" + nonce;
  }
  if (!claims.empty()) {
    url += "&claims=" + utility::url_encode(claims);
  }

  return url;
}

void application::on_authenticate(authenticate_event callback) {
  on_authenticate_ = std::move(callback);
}

std::shared_ptr<session> application::create_session(
    const std::string &user_id, const std::string &login,
    const std::string &access_token, const std::string &id_token,
    uint64_t expires_in, const std::string &refresh_token) {
  if (!socketengine) {
    throw std::runtime_error(
        "application::start() must be called before create_session()");
  }

  auto new_session = std::make_shared<session>(
      this, user_id, login, access_token, id_token, expires_in, refresh_token);
  new_session->connect();

  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_[user_id] = new_session;
  }

  return new_session;
}

std::shared_ptr<session> application::get_session(
    const std::string &user_id) const {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto                        it = sessions_.find(user_id);
  return it == sessions_.end() ? nullptr : it->second;
}

std::vector<std::shared_ptr<session>> application::get_sessions() const {
  std::lock_guard<std::mutex>           lock(sessions_mutex_);
  std::vector<std::shared_ptr<session>> out;
  out.reserve(sessions_.size());
  for (const auto &[id, session] : sessions_) {
    out.push_back(session);
  }
  return out;
}

void application::remove_session(const std::string &user_id) {
  std::shared_ptr<session> session;
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto                        it = sessions_.find(user_id);
    if (it == sessions_.end()) {
      return;
    }
    session = std::move(it->second);
    sessions_.erase(it);
  }

  /* Holds the last reference until the next event loop tick, in case a
   * hook on this session is what called remove_session(). */
  defer([session]() {});
}

void application::get_app_access_token(app_token_event callback) {
  if (client_secret_.empty()) {
    log(ll_error, "get_app_access_token() called without a client_secret");
    if (callback) {
      callback(false, "", 0);
    }
    return;
  }

  std::string body = "client_id=" + utility::url_encode(client_id_) +
                     "&client_secret=" + utility::url_encode(client_secret_) +
                     "&grant_type=client_credentials";
  http_headers headers {
      {"Content-Type", "application/x-www-form-urlencoded"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it     = std::make_unique<https_client>(
      this, "id.twitch.tv", 443, "/oauth2/token", "POST", body, headers, false,
      10, "1.1", [this, it, callback](https_client *c) {
        bool        ok = (c->get_status() == 200);
        std::string token;
        uint64_t    expires_in = 0;
        if (ok) {
          json body_json = json::parse(c->get_content(), nullptr, false);
          if (!body_json.is_discarded() && body_json.is_object()) {
            token      = body_json.value("access_token", "");
            expires_in = body_json.value("expires_in", (uint64_t) 0);
          }
          ok = !token.empty();
        }
        if (callback) {
          callback(ok, token, expires_in);
        }
        defer([this, it]() { pending_app_requests_.erase(it); });
      });
}

void application::refresh_user_token(const std::string  &refresh_token,
                                     refresh_token_event callback) {
  if (client_secret_.empty()) {
    log(ll_error, "refresh_user_token() called without a client_secret");
    if (callback) {
      callback(false, "", "", 0);
    }
    return;
  }

  std::string body = "grant_type=refresh_token&refresh_token=" +
                     utility::url_encode(refresh_token) +
                     "&client_id=" + utility::url_encode(client_id_) +
                     "&client_secret=" + utility::url_encode(client_secret_);
  http_headers headers {
      {"Content-Type", "application/x-www-form-urlencoded"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it     = std::make_unique<https_client>(
      this, "id.twitch.tv", 443, "/oauth2/token", "POST", body, headers, false,
      10, "1.1", [this, it, callback](https_client *c) {
        bool        ok = (c->get_status() == 200);
        std::string access_token;
        std::string new_refresh_token;
        uint64_t    expires_in = 0;
        if (ok) {
          json body_json = json::parse(c->get_content(), nullptr, false);
          if (!body_json.is_discarded() && body_json.is_object()) {
            access_token      = body_json.value("access_token", "");
            new_refresh_token = body_json.value("refresh_token", "");
            expires_in        = body_json.value("expires_in", (uint64_t) 0);
          }
          ok = !access_token.empty();
        }
        if (callback) {
          callback(ok, access_token, new_refresh_token, expires_in);
        }
        defer([this, it]() { pending_app_requests_.erase(it); });
      });
}

void application::handle_auth_request(http_server_request *request) {
  std::string full_path = request->get_path();
  auto        qpos      = full_path.find('?');
  std::string query =
      qpos == std::string::npos ? "" : full_path.substr(qpos + 1);

  if (query.empty()) {
    request->set_status(200)
        .set_response_header("Content-Type", "text/html")
        .set_response_body(fragment_forward_page);
    return;
  }

  auto params = utility::parse_query_string(query);

  if (params.count("error")) {
    log(ll_warning, "Twitch OAuth error: " + params["error"] + " - " +
                        params["error_description"]);
    request->set_status(200)
        .set_response_header("Content-Type", "text/html")
        .set_response_body(auth_failure_page);
    return;
  }

  if (!params.count("access_token") || params.count("state") == 0 ||
      params["state"] != oauth_state_) {
    log(ll_warning, "Twitch OAuth redirect missing token or state mismatch");
    request->set_status(400)
        .set_response_header("Content-Type", "text/html")
        .set_response_body(auth_failure_page);
    return;
  }

  std::string access_token = params["access_token"];
  std::string id_token     = params.count("id_token") ? params["id_token"] : "";
  uint64_t    expires_in =
      params.count("expires_in") ? std::stoull(params["expires_in"]) : 0;
  std::string scope_str = params.count("scope") ? params["scope"] : "";

  request->set_status(200)
      .set_response_header("Content-Type", "text/html")
      .set_response_body(auth_success_page);

  resolve_authenticated_user(access_token, id_token, expires_in, scope_str);
}

void application::resolve_authenticated_user(const std::string &access_token,
                                             const std::string &id_token,
                                             uint64_t           expires_in,
                                             const std::string &scope) {
  http_headers headers {
      {"Authorization", "Bearer " + access_token},
      {"Client-Id",     client_id_              },
  };
  user_lookup_ = std::make_unique<https_client>(
      this, "api.twitch.tv", 443, "/helix/users", "GET", "", headers, false, 10,
      "1.1",
      [this, access_token, id_token, expires_in, scope](https_client *c) {
        if (c->get_status() != 200) {
          log(ll_error,
              "Failed to look up authenticated user, Helix returned "
              "status " +
                  std::to_string(c->get_status()));
          return;
        }
        json body_json = json::parse(c->get_content(), nullptr, false);
        if (body_json.is_discarded() || !body_json.contains("data") ||
            !body_json["data"].is_array() || body_json["data"].empty()) {
          log(ll_error, "Could not find user id/login in Helix response");
          return;
        }
        const json &user    = body_json["data"][0];
        std::string user_id = user.value("id", "");
        std::string login   = user.value("login", "");
        if (user_id.empty() || login.empty()) {
          log(ll_error, "Could not find user id/login in Helix response");
          return;
        }

        (void) scope;
        auto session =
            create_session(user_id, login, access_token, id_token, expires_in);
        if (on_authenticate_) {
          on_authenticate_(*session);
        }
      });
}

}// namespace tpp
