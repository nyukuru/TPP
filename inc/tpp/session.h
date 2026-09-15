#pragma once

#include <cstdint>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "tpp/export.h"
#include "tpp/user.h"

namespace tpp {

class application;
class eventsub_client;
class https_client;
class oneshot_timer;

/**
 * @brief Fired for every chat message received in this session's channel.
 */
using chat_message_event = std::function<void(
    const user &broadcaster, const user &chatter, const std::string &message)>;

/**
 * @brief Represents one Twitch user who has completed an OAuth flow for a
 * tpp::application, parented by that application. Owns the EventSub
 * connection and Helix API calls made on this user's behalf, event hooks
 * such as on_chat_message, and the user's token lifecycle - tracking when
 * the access token expires and, if given a refresh token, rotating it
 * automatically shortly before expiry.
 */
class TPP_EXPORT session : public std::enable_shared_from_this<session> {
  application *owner_;
  std::string  user_id_;
  std::string  login_;
  std::string  access_token_;
  std::string  id_token_;
  std::string  refresh_token_;
  time_t       token_expires_at_ {0};

  std::unique_ptr<eventsub_client> eventsub_;
  std::unique_ptr<oneshot_timer>   rotation_timer_;

  std::mutex                               pending_requests_mutex_;
  std::list<std::unique_ptr<https_client>> pending_requests_;

  chat_message_event on_chat_message_ {};

  void wire_eventsub_callbacks(eventsub_client *client);
  void on_eventsub_welcome(const std::string &session_id);
  void on_eventsub_reconnect(const std::string &reconnect_url);
  void handle_notification(const std::string &subscription_type,
                           const std::string &event_json);
  void subscribe_chat_messages(const std::string &session_id);

  void helix_post(const std::string &path, const std::string &body,
                  std::function<void(https_client *)> on_done);

  /**
   * @brief Records a new access token expiry and reschedules rotation.
   * @param expires_in seconds from now until the current access_token_
   * expires; 0 means unknown
   */
  void set_token_expiry(uint64_t expires_in);

  /**
   * @brief Schedules rotation_timer_ to fire shortly before
   * token_expires_at_.
   */
  void schedule_token_rotation();

  /**
   * @brief Exchanges refresh_token_ for a new access token and
   * reschedules rotation for the result.
   */
  void refresh_access_token();

 public:
  /**
   * @param owner owning application; must outlive this session
   * @param user_id Twitch numeric user ID of the authenticated user
   * @param login Twitch login name of the authenticated user
   * @param access_token user access token
   * @param id_token optional OIDC ID token
   * @param expires_in optional, seconds until access_token expires
   * @param refresh_token optional refresh token
   */
  session(application *owner, std::string user_id, std::string login,
          std::string access_token, std::string id_token = "",
          uint64_t expires_in = 0, std::string refresh_token = "");

  ~session();

  session(const session &)            = delete;
  session &operator=(const session &) = delete;
  session(session &&)                 = delete;
  session &operator=(session &&)      = delete;

  [[nodiscard]] application &get_application() const noexcept;

  [[nodiscard]] const std::string &get_client_id() const noexcept;

  [[nodiscard]] const std::string &get_user_id() const noexcept;

  [[nodiscard]] const std::string &get_login() const noexcept;

  [[nodiscard]] const std::string &get_access_token() const noexcept;

  [[nodiscard]] const std::string &get_id_token() const noexcept;

  /**
   * @brief True if this session has a refresh token.
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
   * @brief Opens this session's EventSub WebSocket connection, and
   * schedules automatic token rotation if a refresh token was provided.
   * Called automatically by application::create_session().
   * @note Must be called on a session owned by a shared_ptr.
   */
  void connect();

  /**
   * @brief Sets the callback fired for every chat message received in
   * this user's channel. Creates the underlying "channel.chat.message"
   * EventSub subscription if the owning application's intents include
   * tpp::i_chat_messages.
   */
  void on_chat_message(chat_message_event callback);

  /**
   * @brief Sends a chat message as this user.
   * @param message message text
   * @param broadcaster_id channel to send to; defaults to this user's own
   * channel (get_user_id())
   */
  void send_message(const std::string &message,
                    const std::string &broadcaster_id = "");

  /**
   * @brief Delivers a chat message notification to the registered hook.
   */
  void dispatch_chat_message(const user &broadcaster, const user &chatter,
                             const std::string &message) const;
};

}// namespace tpp
