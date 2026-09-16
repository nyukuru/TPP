#include <internal/utility.h>
#include <openssl/rand.h>
#include <tpp/auth_server.h>
#include <tpp/conduit.h>
#include <tpp/http_server.h>
#include <tpp/https_client.h>

#include <nlohmann/json.hpp>
#include <stdexcept>

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

auth_server::auth_server(conduit *owner, uint16_t redirect_port) : owner_(owner), redirect_port_(redirect_port) {
  if (redirect_port == 0) {
    throw std::invalid_argument("Redirect port cannot be zero");
  }
  redirect_uri_ = "http://localhost:" + std::to_string(redirect_port);
}

auth_server::~auth_server() {
  stop();
}

void auth_server::start() {
  if (!owner_->socketengine) {
    throw std::runtime_error("conduit::start() must be called before auth_server::start()");
  }
  server_ = std::make_unique<http_server>(owner_, "127.0.0.1", redirect_port_, [this](http_server_request *request) { handle_auth_request(request); });
}

void auth_server::stop() {
  user_lookup_.reset();
  server_.reset();
}

bool auth_server::is_running() const noexcept {
  return server_ != nullptr;
}

std::string auth_server::generate_auth_url(const scope &scopes, const std::string &claims) {
  oauth_state_ = random_hex_nonce();

  std::string response_type = "token";
  std::string nonce;
  if (scopes.has(scope::s_openid)) {
    response_type = "token+id_token";
    nonce = random_hex_nonce();
  }

  std::string url =
      "https://id.twitch.tv/oauth2/authorize"
      "?response_type=" +
      response_type + "&client_id=" + utility::url_encode(owner_->get_client_id()) + "&redirect_uri=" + utility::url_encode(redirect_uri_) +
      "&scope=" + utility::url_encode(scopes.to_string()) + "&state=" + oauth_state_;

  if (!nonce.empty()) {
    url += "&nonce=" + nonce;
  }
  if (!claims.empty()) {
    url += "&claims=" + utility::url_encode(claims);
  }

  return url;
}

void auth_server::handle_auth_request(http_server_request *request) {
  std::string full_path = request->get_path();
  auto qpos = full_path.find('?');
  std::string query = qpos == std::string::npos ? "" : full_path.substr(qpos + 1);

  if (query.empty()) {
    request->set_status(200).set_response_header("Content-Type", "text/html").set_response_body(fragment_forward_page);
    return;
  }

  auto params = utility::parse_query_string(query);

  if (params.count("error")) {
    owner_->log(ll_warning, "Twitch OAuth error: " + params["error"] + " - " + params["error_description"]);
    request->set_status(200).set_response_header("Content-Type", "text/html").set_response_body(auth_failure_page);
    return;
  }

  if (!params.count("access_token") || params.count("state") == 0 || params["state"] != oauth_state_) {
    owner_->log(ll_warning, "Twitch OAuth redirect missing token or state mismatch");
    request->set_status(400).set_response_header("Content-Type", "text/html").set_response_body(auth_failure_page);
    return;
  }

  std::string access_token = params["access_token"];
  std::string id_token = params.count("id_token") ? params["id_token"] : "";
  uint64_t expires_in = params.count("expires_in") ? std::stoull(params["expires_in"]) : 0;
  std::string scope_str = params.count("scope") ? params["scope"] : "";

  request->set_status(200).set_response_header("Content-Type", "text/html").set_response_body(auth_success_page);

  resolve_authenticated_user(access_token, id_token, expires_in, scope_str);
}

void auth_server::resolve_authenticated_user(const std::string &access_token, const std::string &id_token, uint64_t expires_in, const std::string &scope) {
  http_headers headers {
      {"Authorization", "Bearer " + access_token},
      {"Client-Id", owner_->get_client_id()},
  };
  user_lookup_ = std::make_unique<https_client>(
      owner_, "api.twitch.tv", 443, "/helix/users", "GET", "", headers, false, 10, "1.1", [this, access_token, id_token, expires_in, scope](https_client *c) {
        if (c->get_status() != 200) {
          owner_->log(ll_error,
                      "Failed to look up authenticated user, Helix returned "
                      "status " +
                          std::to_string(c->get_status()));
          return;
        }
        json body_json = json::parse(c->get_content(), nullptr, false);
        if (body_json.is_discarded() || !body_json.contains("data") || !body_json["data"].is_array() || body_json["data"].empty()) {
          owner_->log(ll_error, "Could not find user id/login in Helix response");
          return;
        }
        const json &user = body_json["data"][0];
        std::string user_id = user.value("id", "");
        std::string login = user.value("login", "");
        if (user_id.empty() || login.empty()) {
          owner_->log(ll_error, "Could not find user id/login in Helix response");
          return;
        }

        (void) scope;
        auto consumer_ptr = owner_->create_consumer(user_id, login, access_token, id_token, expires_in);
        on_authenticate.call(consumer_ptr);
      });
}

}// namespace tpp
