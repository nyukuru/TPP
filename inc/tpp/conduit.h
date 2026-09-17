#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "tpp/dispatcher.h"
#include "tpp/enums.h"
#include "tpp/event.h"
#include "tpp/event_router.h"
#include "tpp/export.h"
#include "tpp/intents.h"
#include "tpp/socketengine.h"
#include "tpp/timer.h"
#include "tpp/user.h"

namespace tpp {

class https_client;
class eventsub_client;
class consumer;

/**
 * @brief Fired with the result of conduit::get_app_access_token().
 */
using app_token_event = std::function<void(bool success, const std::string &access_token, uint64_t expires_in)>;

/**
 * @brief Fired with the result of conduit::refresh_user_token().
 */
using refresh_token_event = std::function<void(bool success, const std::string &access_token, const std::string &refresh_token, uint64_t expires_in)>;

/**
 * @brief The top level object of a T++ program, analogous to DPP's
 * dpp::cluster. Represents one registered Twitch application (a
 * client_id, optionally paired with its client_secret), owns the shared
 * IO event loop, and owns the EventSub Conduit - the pool of
 * eventsub_client WebSocket shard connections Twitch load-balances every
 * tracked consumer's subscriptions across. A conduit and its shards
 * belong to the client_id, not to any one consumer.
 *
 * The Twitch OIDC implicit grant flow itself is not this class's
 * concern - construct a separate tpp::auth_server (pointed at this
 * conduit) if you need it, and drive its start()/stop() yourself; a
 * conduit neither owns nor knows about one. Each user who authenticates,
 * however that happens, gets their own tpp::consumer, parented by this
 * conduit via create_consumer(), though creating one alone does not
 * track it.
 *
 * Every notification a shard's eventsub_client resolves to a registered
 * handler and a tracked consumer is queued onto this conduit's dispatch
 * thread pool (see enqueue_dispatch()) rather than invoked inline on the
 * IO thread that received it - the on_chat_message-style event_router_t
 * hooks below fire from one of those worker threads.
 */
class TPP_EXPORT conduit {
 private:
  std::string client_id_;
  std::string client_secret_;

  intent intents_;
  std::atomic<bool> running_ {false};
  std::unique_ptr<std::thread> worker_thread_;

  mutable std::mutex consumers_mutex_;
  std::unordered_map<std::string, std::shared_ptr<consumer>> consumers_;

  /* Timer subsystem state, see tpp/timer.h */
  timer_next_t next_timer;
  timers_deleted_t deleted_timers;
  std::mutex timer_guard;

  void worker_loop();

  std::mutex deferred_mutex_;
  std::vector<std::function<void()>> deferred_;

  void run_deferred();

  std::list<std::unique_ptr<https_client>> pending_app_requests_;

  std::string app_access_token_;
  time_t app_access_token_expires_at_ {0};
  std::unique_ptr<oneshot_timer> app_token_rotation_timer_;

  void ensure_app_access_token(std::function<void(bool)> callback);
  void schedule_app_token_rotation();
  void refresh_app_token();

  uint16_t requested_shard_count_ {0};
  std::string conduit_id_;

  mutable std::mutex conduit_mutex_;
  bool conduit_ready_ {false};
  std::vector<std::unique_ptr<eventsub_client>> shards_;

  struct pending_conduit_subscription_t {
    std::weak_ptr<consumer> sess;
    event e;
  };
  std::vector<pending_conduit_subscription_t> pending_conduit_subscriptions_;

  void create_conduit(uint16_t shard_count, std::function<void(bool)> callback);
  void open_shards(uint16_t shard_count);
  void wire_shard_callbacks(eventsub_client *client, uint16_t shard_id);
  void patch_shard_transport(uint16_t shard_id, const std::string &session_id);
  void do_subscribe_for_consumer(const std::shared_ptr<consumer> &c, const event &e);

  /* Per-consumer token lifecycle and Helix calls - conduit is a friend of
   * consumer so these can reach its private token/request-list state
   * directly, mirroring how subscribe_for_consumer/do_subscribe_for_consumer
   * above already own the subscription side of a consumer's lifecycle.
   * Only refresh_access_token()/set_token_expiry()/helix_post() are
   * called solely from within conduit.cpp; schedule_token_rotation() is
   * public below since consumer::schedule_token_rotation() forwards to
   * it. */
  void refresh_access_token(consumer *c);
  void set_token_expiry(consumer *c, uint64_t expires_in);
  void helix_post(consumer *c, const std::string &path, const std::string &body, std::function<void(https_client *)> on_done);

  /* Dispatch thread pool: runs handler->handle() for notifications an
   * eventsub_client shard has resolved to a registered handler and a
   * tracked consumer, off the IO thread. */
  static constexpr size_t DISPATCH_THREAD_COUNT = 4;
  std::vector<std::thread> dispatch_threads_;
  std::mutex dispatch_mutex_;
  std::condition_variable dispatch_cv_;
  std::deque<std::function<void()>> dispatch_queue_;
  bool dispatch_stop_ {false};

  void dispatch_worker_loop();

 public:
  /**
   * @brief Socket engine driving all IO for this conduit.
   * @note stop() resets this to null. Any ssl_connection-derived object
   * you constructed directly against this conduit, other than one owned
   * by a consumer, must be destroyed before calling stop().
   */
  std::unique_ptr<socket_engine_base> socketengine;

  /**
   * @brief Fired whenever a socket managed by the socket engine is closed.
   */
  event_router_t<socket_close_t> on_socket_close;

  /**
   * @brief Fired for every "channel.chat.message" notification resolved
   * to a tracked consumer, once subscribed via consumer::subscribe() with
   * tpp::event::channel_chat_message(). Fires for every subscribed
   * consumer indiscriminately, from a dispatch thread pool worker -
   * filter on chat_message_t::from (the consumer the event arrived for)
   * if you only care about one.
   */
  event_router_t<chat_message_t> on_chat_message;

  /**
   * @brief Fired for every EventSub notification resolved to a tracked
   * consumer, one event_router_t per subscription type - see each
   * payload struct in tpp/dispatcher.h for the matching
   * tpp::event::xxx() subscription builder. Mirrors DPP's on_message_create
   * -style hooks on dpp::cluster.
   */
  event_router_t<automod_message_hold_t> on_automod_message_hold;
  event_router_t<automod_message_update_t> on_automod_message_update;
  event_router_t<automod_settings_update_t> on_automod_settings_update;
  event_router_t<automod_terms_update_t> on_automod_terms_update;
  event_router_t<channel_update_t> on_channel_update;
  event_router_t<channel_follow_t> on_channel_follow;
  event_router_t<channel_ad_break_begin_t> on_channel_ad_break_begin;
  event_router_t<channel_chat_clear_t> on_channel_chat_clear;
  event_router_t<channel_chat_clear_user_messages_t> on_channel_chat_clear_user_messages;
  event_router_t<channel_chat_message_delete_t> on_channel_chat_message_delete;
  event_router_t<channel_chat_notification_t> on_channel_chat_notification;
  event_router_t<channel_chat_settings_update_t> on_channel_chat_settings_update;
  event_router_t<channel_chat_user_message_hold_t> on_channel_chat_user_message_hold;
  event_router_t<channel_chat_user_message_update_t> on_channel_chat_user_message_update;
  event_router_t<channel_shared_chat_begin_t> on_channel_shared_chat_begin;
  event_router_t<channel_shared_chat_update_t> on_channel_shared_chat_update;
  event_router_t<channel_shared_chat_end_t> on_channel_shared_chat_end;
  event_router_t<channel_subscribe_t> on_channel_subscribe;
  event_router_t<channel_subscription_end_t> on_channel_subscription_end;
  event_router_t<channel_subscription_gift_t> on_channel_subscription_gift;
  event_router_t<channel_subscription_message_t> on_channel_subscription_message;
  event_router_t<channel_cheer_t> on_channel_cheer;
  event_router_t<channel_raid_t> on_channel_raid;
  event_router_t<channel_ban_t> on_channel_ban;
  event_router_t<channel_unban_t> on_channel_unban;
  event_router_t<channel_unban_request_create_t> on_channel_unban_request_create;
  event_router_t<channel_unban_request_resolve_t> on_channel_unban_request_resolve;
  event_router_t<channel_moderate_t> on_channel_moderate;
  event_router_t<channel_moderator_add_t> on_channel_moderator_add;
  event_router_t<channel_moderator_remove_t> on_channel_moderator_remove;
  event_router_t<channel_guest_star_session_begin_t> on_channel_guest_star_session_begin;
  event_router_t<channel_guest_star_session_end_t> on_channel_guest_star_session_end;
  event_router_t<channel_guest_star_guest_update_t> on_channel_guest_star_guest_update;
  event_router_t<channel_guest_star_settings_update_t> on_channel_guest_star_settings_update;
  event_router_t<channel_channel_points_automatic_reward_redemption_add_t> on_channel_channel_points_automatic_reward_redemption_add;
  event_router_t<channel_channel_points_custom_reward_add_t> on_channel_channel_points_custom_reward_add;
  event_router_t<channel_channel_points_custom_reward_update_t> on_channel_channel_points_custom_reward_update;
  event_router_t<channel_channel_points_custom_reward_remove_t> on_channel_channel_points_custom_reward_remove;
  event_router_t<channel_channel_points_custom_reward_redemption_add_t> on_channel_channel_points_custom_reward_redemption_add;
  event_router_t<channel_channel_points_custom_reward_redemption_update_t> on_channel_channel_points_custom_reward_redemption_update;
  event_router_t<channel_poll_begin_t> on_channel_poll_begin;
  event_router_t<channel_poll_progress_t> on_channel_poll_progress;
  event_router_t<channel_poll_end_t> on_channel_poll_end;
  event_router_t<channel_prediction_begin_t> on_channel_prediction_begin;
  event_router_t<channel_prediction_progress_t> on_channel_prediction_progress;
  event_router_t<channel_prediction_lock_t> on_channel_prediction_lock;
  event_router_t<channel_prediction_end_t> on_channel_prediction_end;
  event_router_t<channel_suspicious_user_message_t> on_channel_suspicious_user_message;
  event_router_t<channel_suspicious_user_update_t> on_channel_suspicious_user_update;
  event_router_t<channel_vip_add_t> on_channel_vip_add;
  event_router_t<channel_vip_remove_t> on_channel_vip_remove;
  event_router_t<channel_warning_acknowledge_t> on_channel_warning_acknowledge;
  event_router_t<channel_warning_send_t> on_channel_warning_send;
  event_router_t<channel_charity_campaign_donate_t> on_channel_charity_campaign_donate;
  event_router_t<channel_charity_campaign_start_t> on_channel_charity_campaign_start;
  event_router_t<channel_charity_campaign_progress_t> on_channel_charity_campaign_progress;
  event_router_t<channel_charity_campaign_stop_t> on_channel_charity_campaign_stop;
  event_router_t<channel_goal_begin_t> on_channel_goal_begin;
  event_router_t<channel_goal_progress_t> on_channel_goal_progress;
  event_router_t<channel_goal_end_t> on_channel_goal_end;
  event_router_t<channel_hype_train_begin_t> on_channel_hype_train_begin;
  event_router_t<channel_hype_train_progress_t> on_channel_hype_train_progress;
  event_router_t<channel_hype_train_end_t> on_channel_hype_train_end;
  event_router_t<channel_shield_mode_begin_t> on_channel_shield_mode_begin;
  event_router_t<channel_shield_mode_end_t> on_channel_shield_mode_end;
  event_router_t<channel_shoutout_create_t> on_channel_shoutout_create;
  event_router_t<channel_shoutout_receive_t> on_channel_shoutout_receive;
  event_router_t<conduit_shard_disabled_t> on_conduit_shard_disabled;
  event_router_t<drop_entitlement_grant_t> on_drop_entitlement_grant;
  event_router_t<extension_bits_transaction_create_t> on_extension_bits_transaction_create;
  event_router_t<stream_online_t> on_stream_online;
  event_router_t<stream_offline_t> on_stream_offline;
  event_router_t<user_authorization_grant_t> on_user_authorization_grant;
  event_router_t<user_authorization_revoke_t> on_user_authorization_revoke;
  event_router_t<user_update_t> on_user_update;
  event_router_t<user_whisper_message_t> on_user_whisper_message;

  /**
   * @brief Queues a function to run on the next iteration of the IO event
   * loop.
   */
  void defer(std::function<void()> fn);

  /**
   * @brief Queues a unit of work to run on this conduit's dispatch thread
   * pool, off the IO thread. Used internally by eventsub_client to invoke
   * event_handler::handle() for a resolved notification; exposed in case
   * calling code wants to offload work the same way.
   */
  void enqueue_dispatch(std::function<void()> work);

  /**
   * @param client_id Twitch application client ID
   * @param client_secret Optional. Required by get_app_access_token(),
   * refresh_user_token(), and open_conduit() (and therefore by
   * consumer::subscribe(), which routes through this conduit).
   * @param intent_flags EventSub subscription categories this conduit
   * intends to use. Informational only; consumers do not subscribe to
   * anything automatically - call consumer::subscribe() explicitly.
   * @param shard_count If nonzero, start() calls open_conduit(shard_count)
   * on your behalf once it is running, so you do not need to call it
   * yourself. Errors are only logged, since there is no callback to
   * report them to this way - call open_conduit() directly if you need
   * one. 0 (the default) leaves opening the conduit entirely up to you.
   */
  explicit conduit(const std::string &client_id, const std::string &client_secret = "", intent intent_flags = intent(i_chat_messages),
                   uint16_t shard_count = 0);

  ~conduit();

  conduit(const conduit &) = delete;
  conduit &operator=(const conduit &) = delete;
  conduit(conduit &&) = delete;
  conduit &operator=(conduit &&) = delete;

  /**
   * @brief Starts the conduit: the socket engine and its dispatch thread
   * pool.
   * @param wait if true, blocks until stop() is called from another
   * thread; if false, returns immediately (see join())
   * @throw std::runtime_error if already running
   */
  void start(bool wait = true);

  /**
   * @brief Blocks until the conduit stops running. Only useful after
   * start(false); start(true) already blocks until stop() is called.
   */
  void join();

  void stop();

  [[nodiscard]] bool is_running() const noexcept;

  [[nodiscard]] const std::string &get_client_id() const noexcept;

  [[nodiscard]] intent get_intents() const noexcept;

  /**
   * @brief Constructs a consumer directly from an already-obtained access
   * token, without going through the local OAuth redirect server. The
   * consumer is not tracked by this conduit - call add_consumer() if you
   * want it to be.
   * @param user_id Twitch numeric user ID of the authenticated user
   * @param login Twitch login name of the authenticated user
   * @param access_token user access token
   * @param id_token optional OIDC ID token
   * @param expires_in optional, seconds until access_token expires
   * @param refresh_token optional refresh token. If given, the consumer
   * rotates access_token on its own shortly before expires_in runs out
   * @return the newly created consumer
   */
  std::shared_ptr<consumer> create_consumer(const std::string &user_id, const std::string &login, const std::string &access_token,
                                            const std::string &id_token = "", uint64_t expires_in = 0, const std::string &refresh_token = "");

  /**
   * @brief Starts tracking a consumer, so it can be found with
   * get_consumer()/get_consumers(), is torn down by remove_consumer() or
   * stop(), and has its access token rotated automatically if it has a
   * refresh token. Replaces any existing tracked consumer for the same
   * user ID.
   * @param c the consumer to track
   */
  void add_consumer(std::shared_ptr<consumer> c);

  /**
   * @brief Looks up a tracked consumer by Twitch user ID.
   * @return the consumer, or nullptr if no such consumer is tracked
   */
  [[nodiscard]] std::shared_ptr<consumer> get_consumer(const std::string &user_id) const;

  /**
   * @brief Every consumer currently tracked by this conduit.
   */
  [[nodiscard]] std::vector<std::shared_ptr<consumer>> get_consumers() const;

  /**
   * @brief Stops tracking a consumer. Does not affect this conduit's
   * shards, which are shared with every other tracked consumer.
   */
  void remove_consumer(const std::string &user_id);

  /**
   * @brief Creates a Twitch EventSub Conduit and opens `shard_count`
   * eventsub_client WebSocket connections for it. A conduit belongs to
   * this client_id, not to any one consumer - Twitch load-balances every
   * tracked consumer's subscriptions across its shards. Requires
   * client_secret and start() to have been called first.
   * @param shard_count number of WebSocket shard connections to open
   * @param callback optional, called with true once the conduit exists
   * and is ready to accept subscriptions, or false on failure
   * @throw std::runtime_error if start() has not been called
   * @throw std::invalid_argument if shard_count is 0
   */
  void open_conduit(uint16_t shard_count, std::function<void(bool)> callback = {});

  /**
   * @brief The ID of this conduit's EventSub Conduit, or empty if
   * open_conduit() has not been called or has not yet completed.
   */
  [[nodiscard]] const std::string &get_conduit_id() const noexcept;

  /**
   * @brief Current number of shard connections open on this conduit's
   * EventSub Conduit. Always reflects live state, safe to call from any
   * thread - it climbs as open_conduit()/open_shards() add shards and
   * drops back to 0 once stop() or delete_conduit() tears them down.
   */
  [[nodiscard]] uint16_t get_shard_count() const noexcept;

  /**
   * @brief Looks up a shard connection by its ID, mirroring DPP's
   * dpp::cluster::get_shard() - used by event_dispatch_t::from() to
   * resolve the eventsub_client the event arrived on. Looked up by ID
   * rather than storing the pointer directly, since a shard's
   * eventsub_client is replaced (not just reconnected in place) whenever
   * it reconnects - see wire_shard_callbacks().
   * @param shard_id shard ID, e.g. from event_dispatch_t::shard
   * @return the shard, or nullptr if shard_id is out of range
   */
  [[nodiscard]] eventsub_client *get_shard(uint16_t shard_id) const;

  /**
   * @brief Deletes this conduit's EventSub Conduit via the Twitch API,
   * along with every subscription attached to it, and closes this
   * conduit's shards. A conduit otherwise outlives the process that
   * created it - Twitch does not clean these up on its own - so anything
   * that calls open_conduit() repeatedly against the same client_id
   * (tests, in particular) should call this once done, or it will
   * eventually hit Twitch's per-client_id conduit limit. No-op if
   * open_conduit() has not been called or has not yet completed.
   * @param callback optional, called with true once deleted (or if there
   * was nothing to delete), or false on failure
   */
  void delete_conduit(std::function<void(bool)> callback = {});

  /**
   * @brief Creates an EventSub subscription on behalf of a consumer. If
   * the conduit is not ready yet, the subscription is created once it is.
   * @param c the consumer the subscription is made on behalf of
   * @param e the subscription to create
   */
  void subscribe_for_consumer(std::shared_ptr<consumer> c, const event &e);

  /**
   * @brief Same as subscribe_for_consumer(std::shared_ptr<consumer>, const
   * event &), but looks the consumer up by identity - handy when all you
   * have is a tpp::user from an event payload (e.g. channel_ban_t::
   * moderator). No-op, logged at ll_debug, if u.id is not a tracked
   * consumer.
   */
  void subscribe_for_consumer(const user &u, const event &e);

  /**
   * @brief Schedules automatic access token rotation for a tracked
   * consumer shortly before its current access token expires, if it has
   * a refresh token. Called automatically by add_consumer(); exposed
   * because consumer::schedule_token_rotation() forwards to it.
   * @note c must be owned by a shared_ptr.
   */
  void schedule_token_rotation(consumer *c);

  /**
   * @brief Sends a chat message as a tracked consumer.
   * @param c the consumer to send as
   * @param message message text
   * @param broadcaster_id channel to send to; defaults to c's own channel
   */
  void send_message(consumer *c, const std::string &message, const std::string &broadcaster_id = "");

  /**
   * @brief Same as send_message(consumer *, const std::string &, const
   * std::string &), but looks the sender up by identity - handy when all
   * you have is a tpp::user from an event payload. No-op, logged at
   * ll_debug, if u.id is not a tracked consumer.
   * @param u the user to send as - must be a tracked consumer
   * @param message message text
   * @param broadcaster_id channel to send to; defaults to u's own channel
   */
  void send_message(const user &u, const std::string &message, const std::string &broadcaster_id = "");

  /**
   * @brief Obtains an app access token via the OAuth2 Client Credentials
   * Grant. Requires client_secret. An app access token is not tied to any
   * user and cannot be used for user-scoped EventSub subscriptions, but
   * works with app-scoped Helix endpoints and client credential
   * validation.
   * @note This is a stateless one-off request - it does not touch the
   * app access token this conduit itself keeps refreshed (see start()).
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
  void refresh_user_token(const std::string &refresh_token, refresh_token_event callback);

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
  timer start_timer(timer_callback_t on_tick, uint64_t frequency, timer_callback_t on_stop = {});

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
