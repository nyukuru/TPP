#pragma once

#include <cstdint>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "tpp/event.h"
#include "tpp/export.h"
#include "tpp/user.h"

namespace tpp {

class conduit;
class https_client;
class oneshot_timer;

/**
 * @brief Represents one Twitch user who has completed an OAuth flow for a
 * tpp::conduit, parented by that conduit - a fancy wrapper around a
 * Twitch user identity with the token lifecycle bolted on: tracking when
 * the access token expires and, if given a refresh token, rotating it
 * automatically shortly before expiry. Event routing and handling belong
 * to eventsub_client, which owns the shard a notification arrives on;
 * this class only owns what is specific to the authorized user
 * themselves - tokens, Helix calls made on their behalf, and the
 * subscribe()/send_message() convenience wrappers.
 */
class TPP_EXPORT consumer : public std::enable_shared_from_this<consumer> {
  /* conduit owns the actual token lifecycle and Helix call machinery
   * (conduit::schedule_token_rotation(), ::refresh_access_token(),
   * ::set_token_expiry(), ::helix_post()) and reaches into the private
   * state below directly to do it; the methods below this class keeps
   * are thin consumer-shaped wrappers that just call through to it. */
  friend class conduit;

  conduit *owner_;
  std::string user_id_;
  std::string login_;
  std::string access_token_;
  std::string id_token_;
  std::string refresh_token_;
  time_t token_expires_at_ {0};

  std::unique_ptr<oneshot_timer> rotation_timer_;

  std::mutex pending_requests_mutex_;
  std::list<std::unique_ptr<https_client>> pending_requests_;

 public:
  /**
   * @param owner owning conduit; must outlive this consumer
   * @param user_id Twitch numeric user ID of the authenticated user
   * @param login Twitch login name of the authenticated user
   * @param access_token user access token
   * @param id_token optional OIDC ID token
   * @param expires_in optional, seconds until access_token expires
   * @param refresh_token optional refresh token
   */
  consumer(conduit *owner, std::string user_id, std::string login, std::string access_token, std::string id_token = "", uint64_t expires_in = 0,
           std::string refresh_token = "");

  ~consumer();

  consumer(const consumer &) = delete;
  consumer &operator=(const consumer &) = delete;
  consumer(consumer &&) = delete;
  consumer &operator=(consumer &&) = delete;

  [[nodiscard]] conduit &get_conduit() const noexcept;

  [[nodiscard]] const std::string &get_client_id() const noexcept;

  [[nodiscard]] const std::string &get_user_id() const noexcept;

  [[nodiscard]] const std::string &get_login() const noexcept;

  [[nodiscard]] const std::string &get_access_token() const noexcept;

  [[nodiscard]] const std::string &get_id_token() const noexcept;

  /**
   * @brief True if this consumer has a refresh token.
   */
  [[nodiscard]] bool has_refresh_token() const noexcept;

  [[nodiscard]] const std::string &get_refresh_token() const noexcept;

  /**
   * @brief Absolute expiry time of the current access token, as a unix
   * timestamp, or 0 if unknown.
   */
  [[nodiscard]] time_t get_token_expires_at() const noexcept;

  /**
   * @brief Seconds remaining until the current access token expires, or 0
   * if unknown or already expired.
   */
  [[nodiscard]] uint64_t get_token_expires_in() const noexcept;

  /**
   * @brief Schedules automatic token rotation shortly before the current
   * access token expires, if this consumer has a refresh token. Called
   * automatically by conduit::add_consumer(). Forwards to
   * conduit::schedule_token_rotation(), which owns the actual logic.
   * @note Must be called on a consumer owned by a shared_ptr.
   */
  void schedule_token_rotation();

  /**
   * @brief Creates an EventSub subscription for this consumer. Forwards
   * to conduit::subscribe_for_consumer(), which owns the actual logic. If
   * the conduit is not ready yet, the subscription is created once it is.
   * @param e the subscription to create
   */
  void subscribe(const event &e);

  /**
   * @brief Sends a chat message as this user. Forwards to
   * conduit::send_message(), which owns the actual logic.
   * @param message message text
   * @param broadcaster_id channel to send to; defaults to this user's own
   * channel (get_user_id())
   */
  void send_message(const std::string &message, const std::string &broadcaster_id = "");
};

}// namespace tpp
