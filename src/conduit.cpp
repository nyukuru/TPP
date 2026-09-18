#include <internal/utility.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>
#include <tpp/https_client.h>

#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string_view>
#include <thread>

namespace tpp {

namespace {

using json = nlohmann::json;

// Twitch reports an EventSub subscription that already exists (e.g. the
// conduit survived a restart and still has it) as a 409 whose message
// looks like "subscription already exists; id=<uuid>" - pulls that id out
// so the duplicate can be treated as a successful, idempotent no-op
// instead of a failure.
std::string extract_existing_subscription_id(const std::string &body) {
  json resp = json::parse(body, nullptr, false);
  std::string message = !resp.is_discarded() ? resp.value("message", "") : body;
  constexpr std::string_view id_marker = "id=";
  auto pos = message.find(id_marker);
  if (pos == std::string::npos) {
    return "";
  }
  return message.substr(pos + id_marker.size());
}

}// namespace

conduit::conduit(const std::string &client_id, const std::string &client_secret, intent intent_flags, uint16_t shard_count)
    : client_id_(client_id), client_secret_(client_secret), intents_(intent_flags), requested_shard_count_(shard_count) {
  if (client_id.empty()) {
    throw std::invalid_argument("Client ID cannot be empty");
  }
}

conduit::~conduit() {
  if (running_.load()) {
    stop();
  }
}

void conduit::start(bool wait) {
  if (running_.load()) {
    throw std::runtime_error("Conduit is already running");
  }

  socketengine = create_socket_engine(this);

  running_.store(true);
  worker_thread_ = std::make_unique<std::thread>(&conduit::worker_loop, this);

  dispatch_threads_.reserve(DISPATCH_THREAD_COUNT);
  for (size_t i = 0; i < DISPATCH_THREAD_COUNT; ++i) {
    dispatch_threads_.emplace_back(&conduit::dispatch_worker_loop, this);
  }

  if (!client_secret_.empty()) {
    ensure_app_access_token([this](bool ok) {
      if (!ok) {
        log(ll_error, "Failed to obtain initial app access token");
      }
    });
  }

  if (requested_shard_count_ > 0) {
    open_conduit(requested_shard_count_, [this](bool ok) {
      if (!ok) {
        log(ll_error, "Failed to open conduit requested at construction");
      }
    });
  }

  if (wait) {
    join();
  }
}

void conduit::join() {
  if (worker_thread_ && worker_thread_->joinable()) {
    worker_thread_->join();
  }
}

void conduit::stop() {
  if (!running_.load()) {
    return;
  }

  running_.store(false);
  join();
  worker_thread_.reset();

  /* Drains work queued while stopping, before it can touch socketengine
   * after that is reset below. */
  run_deferred();

  {
    std::lock_guard<std::mutex> lock(dispatch_mutex_);
    dispatch_stop_ = true;
  }
  dispatch_cv_.notify_all();
  for (auto &t : dispatch_threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
  dispatch_threads_.clear();
  dispatch_stop_ = false;

  {
    std::lock_guard<std::mutex> lock(consumers_mutex_);
    consumers_.clear();
  }
  {
    std::lock_guard<std::mutex> lock(conduit_mutex_);
    conduit_ready_ = false;
    pending_conduit_subscriptions_.clear();
    shards_.clear();
  }
  conduit_id_.clear();

  app_token_rotation_timer_.reset();
  app_access_token_.clear();
  app_access_token_expires_at_ = 0;

  /* Any https_client still in here never got to run its own defer()-based
   * erase, because nothing is polling the socket engine to finish it once
   * the worker thread above has stopped - most commonly one from a
   * get_app_access_token()-family call still in flight. Destroying it here,
   * before socketengine is reset below, is required: ssl_connection's
   * destructor deregisters from socketengine, so destroying these after
   * would touch a socketengine that no longer exists. */
  pending_app_requests_.clear();

  socketengine.reset();
}

bool conduit::is_running() const noexcept {
  return running_.load();
}

const std::string &conduit::get_client_id() const noexcept {
  return client_id_;
}

intent conduit::get_intents() const noexcept {
  return intents_;
}

void conduit::worker_loop() {
  while (running_.load()) {
    run_deferred();
    socketengine->process_events();
  }
}

void conduit::defer(std::function<void()> fn) {
  std::lock_guard<std::mutex> lock(deferred_mutex_);
  deferred_.push_back(std::move(fn));
}

void conduit::run_deferred() {
  std::vector<std::function<void()>> pending;
  {
    std::lock_guard<std::mutex> lock(deferred_mutex_);
    pending.swap(deferred_);
  }
  for (auto &fn : pending) {
    fn();
  }
}

void conduit::enqueue_dispatch(std::function<void()> work) {
  {
    std::lock_guard<std::mutex> lock(dispatch_mutex_);
    dispatch_queue_.push_back(std::move(work));
  }
  dispatch_cv_.notify_one();
}

void conduit::dispatch_worker_loop() {
  while (true) {
    std::function<void()> work;
    {
      std::unique_lock<std::mutex> lock(dispatch_mutex_);
      dispatch_cv_.wait(lock, [this] { return dispatch_stop_ || !dispatch_queue_.empty(); });
      if (dispatch_stop_ && dispatch_queue_.empty()) {
        return;
      }
      work = std::move(dispatch_queue_.front());
      dispatch_queue_.pop_front();
    }
    work();
  }
}

void conduit::log(loglevel severity, const std::string &message) const {
  static constexpr const char *labels[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
  const char *label = (severity >= 0 && severity <= ll_critical) ? labels[severity] : "?";
  std::cerr << "[" << label << "] " << message << std::endl;
}

std::shared_ptr<consumer> conduit::create_consumer(const std::string &user_id, const std::string &login, const std::string &access_token,
                                                   const std::string &id_token, uint64_t expires_in, const std::string &refresh_token) {
  return std::make_shared<consumer>(this, user_id, login, access_token, id_token, expires_in, refresh_token);
}

void conduit::add_consumer(std::shared_ptr<consumer> c) {
  c->schedule_token_rotation();

  std::string user_id = c->get_user_id();
  std::lock_guard<std::mutex> lock(consumers_mutex_);
  consumers_[user_id] = std::move(c);
}

std::shared_ptr<consumer> conduit::get_consumer(const std::string &user_id) const {
  std::lock_guard<std::mutex> lock(consumers_mutex_);
  auto it = consumers_.find(user_id);
  return it == consumers_.end() ? nullptr : it->second;
}

std::vector<std::shared_ptr<consumer>> conduit::get_consumers() const {
  std::lock_guard<std::mutex> lock(consumers_mutex_);
  std::vector<std::shared_ptr<consumer>> out;
  out.reserve(consumers_.size());
  for (const auto &[id, c] : consumers_) {
    out.push_back(c);
  }
  return out;
}

void conduit::remove_consumer(const std::string &user_id) {
  std::shared_ptr<consumer> c;
  {
    std::lock_guard<std::mutex> lock(consumers_mutex_);
    auto it = consumers_.find(user_id);
    if (it == consumers_.end()) {
      return;
    }
    c = std::move(it->second);
    consumers_.erase(it);
  }

  /* Holds the last reference until the next event loop tick, in case a
   * hook on this consumer is what called remove_consumer(). */
  defer([c]() {});
}

void conduit::open_conduit(uint16_t shard_count, std::function<void(bool)> callback) {
  if (!socketengine) {
    throw std::runtime_error("conduit::start() must be called before open_conduit()");
  }
  if (shard_count == 0) {
    throw std::invalid_argument("shard_count must be at least 1");
  }

  ensure_app_access_token([this, shard_count, callback](bool ok) {
    if (!ok) {
      log(ll_error,
          "open_conduit() failed: could not obtain an app "
          "access token");
      if (callback) {
        callback(false);
      }
      return;
    }
    find_or_create_conduit(shard_count, callback);
  });
}

void conduit::find_or_create_conduit(uint16_t shard_count, std::function<void(bool)> callback) {
  helix_get("/helix/eventsub/conduits", [this, shard_count, callback](https_client *c) {
    std::string existing_id;
    if (c && c->get_status() == 200) {
      json resp = json::parse(c->get_content(), nullptr, false);
      if (!resp.is_discarded() && resp.contains("data") && resp["data"].is_array() && !resp["data"].empty()) {
        existing_id = resp["data"][0].value("id", "");
      }
    }

    if (existing_id.empty()) {
      create_conduit(shard_count, callback);
      return;
    }

    log(ll_info, "Reusing existing EventSub conduit " + existing_id);
    finish_opening_conduit(existing_id, shard_count, callback);
  });
}

const std::string &conduit::get_conduit_id() const noexcept {
  return conduit_id_;
}

uint16_t conduit::get_shard_count() const noexcept {
  std::lock_guard<std::mutex> lock(conduit_mutex_);
  return static_cast<uint16_t>(shards_.size());
}

eventsub_client *conduit::get_shard(uint16_t shard_id) const {
  std::lock_guard<std::mutex> lock(conduit_mutex_);
  if (shard_id >= shards_.size()) {
    return nullptr;
  }
  return shards_[shard_id].get();
}

void conduit::delete_conduit(std::function<void(bool)> callback) {
  if (conduit_id_.empty()) {
    if (callback) {
      callback(true);
    }
    return;
  }

  http_headers headers {
      {"Authorization", "Bearer " + app_access_token_},
      {"Client-Id", client_id_},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(
      this, "api.twitch.tv", 443, "/helix/eventsub/conduits?id=" + utility::url_encode(conduit_id_), "DELETE", "", headers, false, 10, "1.1",
      [this, it, callback](https_client *c) {
        bool ok = (c->get_status() == 204);
        if (!ok) {
          log(ll_error, "Failed to delete EventSub conduit, Helix returned status " + std::to_string(c->get_status()) + ": " + c->get_content());
        }
        defer([this, it]() { pending_app_requests_.erase(it); });

        {
          std::lock_guard<std::mutex> lock(conduit_mutex_);
          conduit_ready_ = false;
          shards_.clear();
        }
        conduit_id_.clear();

        if (callback) {
          callback(ok);
        }
      });
}

void conduit::create_conduit(uint16_t shard_count, std::function<void(bool)> callback) {
  json body {{"shard_count", shard_count}};
  http_headers headers {
      {"Authorization", "Bearer " + app_access_token_},
      {"Client-Id", client_id_},
      {"Content-Type", "application/json"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, "/helix/eventsub/conduits", "POST", body.dump(), headers, false, 10, "1.1",
                                       [this, it, shard_count, callback](https_client *c) {
                                         bool ok = (c->get_status() == 200);
                                         std::string new_conduit_id;
                                         if (ok) {
                                           json resp = json::parse(c->get_content(), nullptr, false);
                                           if (!resp.is_discarded() && resp.contains("data") && resp["data"].is_array() && !resp["data"].empty()) {
                                             new_conduit_id = resp["data"][0].value("id", "");
                                           }
                                           ok = !new_conduit_id.empty();
                                         }
                                         if (!ok) {
                                           log(ll_error,
                                               "Failed to create EventSub conduit, Helix returned "
                                               "status " +
                                                   std::to_string(c->get_status()) + ": " + c->get_content());
                                         }
                                         defer([this, it]() { pending_app_requests_.erase(it); });

                                         if (!ok) {
                                           if (callback) {
                                             callback(false);
                                           }
                                           return;
                                         }

                                         finish_opening_conduit(new_conduit_id, shard_count, callback);
                                       });
}

void conduit::finish_opening_conduit(const std::string &id, uint16_t shard_count, std::function<void(bool)> callback) {
  conduit_id_ = id;

  std::vector<pending_conduit_subscription_t> pending;
  std::vector<pending_app_subscription_t> pending_app;
  {
    std::lock_guard<std::mutex> lock(conduit_mutex_);
    conduit_ready_ = true;
    pending.swap(pending_conduit_subscriptions_);
    pending_app.swap(pending_app_subscriptions_);
  }
  for (auto &p : pending) {
    if (auto sess = p.sess.lock()) {
      do_subscribe_for_consumer(sess, p.e);
    }
  }
  for (auto &p : pending_app) {
    do_subscribe_app(p.e, p.callback);
  }

  open_shards(shard_count);

  if (callback) {
    callback(true);
  }
}

void conduit::open_shards(uint16_t shard_count) {
  for (uint16_t i = 0; i < shard_count; ++i) {
    auto client = std::make_unique<eventsub_client>(this);
    wire_shard_callbacks(client.get(), i);
    std::lock_guard<std::mutex> lock(conduit_mutex_);
    shards_.push_back(std::move(client));
  }
}

void conduit::wire_shard_callbacks(eventsub_client *client, uint16_t shard_id) {
  client->shard_id = shard_id;
  client->on_welcome([this, shard_id](const eventsub_welcome_t &w) { patch_shard_transport(shard_id, w.session_id); });
  client->on_reconnect([this, shard_id](const eventsub_reconnect_t &r) {
    log(ll_info, "EventSub shard " + std::to_string(shard_id) + " reconnecting");
    auto new_client = std::make_unique<eventsub_client>(this, r.reconnect_url);
    wire_shard_callbacks(new_client.get(), shard_id);

    /* conduit::defer requires a copy-constructible target. */
    auto shared_client = std::make_shared<std::unique_ptr<eventsub_client>>(std::move(new_client));

    /* Replacing shards_[shard_id] here destroys the client whose own call
     * stack invoked this callback, so it must happen on the next event
     * loop tick. */
    defer([this, shard_id, shared_client]() {
      std::lock_guard<std::mutex> lock(conduit_mutex_);
      if (shard_id < shards_.size()) {
        shards_[shard_id] = std::move(*shared_client);
      }
    });
  });
}

void conduit::patch_shard_transport(uint16_t shard_id, const std::string &session_id) {
  json body {
      {"conduit_id", conduit_id_},
      {"shards", json::array({{{"id", std::to_string(shard_id)}, {"transport", {{"method", "websocket"}, {"session_id", session_id}}}}})},
  };
  http_headers headers {
      {"Authorization", "Bearer " + app_access_token_},
      {"Client-Id", client_id_},
      {"Content-Type", "application/json"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, "/helix/eventsub/conduits/shards", "PATCH", body.dump(), headers, false, 10, "1.1",
                                       [this, it, shard_id](https_client *c) {
                                         if (c->get_status() != 202 && c->get_status() != 200) {
                                           log(ll_error, "Failed to assign EventSub shard " + std::to_string(shard_id) + ", Helix returned status " +
                                                             std::to_string(c->get_status()) + ": " + c->get_content());
                                         }
                                         defer([this, it]() { pending_app_requests_.erase(it); });
                                       });
}

void conduit::subscribe_for_consumer(std::shared_ptr<consumer> c, const event &e) {
  bool ready;
  {
    std::lock_guard<std::mutex> lock(conduit_mutex_);
    ready = conduit_ready_;
    if (!ready) {
      pending_conduit_subscriptions_.push_back({c, e});
    }
  }
  if (ready) {
    do_subscribe_for_consumer(c, e);
  }
}

void conduit::do_subscribe_for_consumer(const std::shared_ptr<consumer> &c, const event &e) {
  json condition = json::object();
  for (const auto &[key, value] : e.condition) {
    condition[key] = value;
  }

  json body {
      {"type", e.type},
      {"version", e.version},
      {"condition", condition},
      {"transport", {{"method", "conduit"}, {"conduit_id", conduit_id_}}},
  };
  http_headers headers {
      {"Authorization", "Bearer " + c->get_access_token()},
      {"Client-Id", client_id_},
      {"Content-Type", "application/json"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, "/helix/eventsub/subscriptions", "POST", body.dump(), headers, false, 10, "1.1",
                                       [this, it, type = e.type, login = c->get_login()](https_client *c) {
                                         int status = c->get_status();
                                         if (status == 409) {
                                           std::string existing_id = extract_existing_subscription_id(c->get_content());
                                           if (!existing_id.empty()) {
                                             log(ll_info, "Reusing existing " + type + " subscription " + existing_id + " for " + login);
                                           } else {
                                             log(ll_error, "Failed to create " + type + " subscription for " + login + ", Helix returned status " +
                                                               std::to_string(status) + ": " + c->get_content());
                                           }
                                         } else if (status != 202) {
                                           log(ll_error, "Failed to create " + type + " subscription for " + login + ", Helix returned status " +
                                                             std::to_string(status) + ": " + c->get_content());
                                         }
                                         defer([this, it]() { pending_app_requests_.erase(it); });
                                       });
}

void conduit::subscribe_for_consumer(const user &u, const event &e) {
  auto c = get_consumer(u.id);
  if (!c) {
    log(ll_debug, "subscribe_for_consumer: " + u.id + " is not a tracked consumer");
    return;
  }
  subscribe_for_consumer(c, e);
}

void conduit::subscribe(const event &e, subscribe_event callback) {
  bool ready;
  {
    std::lock_guard<std::mutex> lock(conduit_mutex_);
    ready = conduit_ready_;
    if (!ready) {
      pending_app_subscriptions_.push_back({e, callback});
    }
  }
  if (ready) {
    do_subscribe_app(e, callback);
  }
}

void conduit::do_subscribe_app(const event &e, subscribe_event callback) {
  ensure_app_access_token([this, e, callback](bool ok) {
    if (!ok) {
      log(ll_error, "Failed to create " + e.type + " subscription: could not obtain an app access token");
      if (callback) {
        callback(false, "");
      }
      return;
    }

    json condition = json::object();
    for (const auto &[key, value] : e.condition) {
      condition[key] = value;
    }

    json body {
        {"type", e.type},
        {"version", e.version},
        {"condition", condition},
        {"transport", {{"method", "conduit"}, {"conduit_id", conduit_id_}}},
    };
    http_headers headers {
        {"Authorization", "Bearer " + app_access_token_},
        {"Client-Id", client_id_},
        {"Content-Type", "application/json"},
    };

    auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
    *it = std::make_unique<https_client>(
        this, "api.twitch.tv", 443, "/helix/eventsub/subscriptions", "POST", body.dump(), headers, false, 10, "1.1",
        [this, it, type = e.type, callback](https_client *c) {
          int status = c->get_status();
          bool ok = (status == 202);
          std::string subscription_id;
          if (ok) {
            json resp = json::parse(c->get_content(), nullptr, false);
            if (!resp.is_discarded() && resp.contains("data") && resp["data"].is_array() && !resp["data"].empty()) {
              subscription_id = resp["data"][0].value("id", "");
            }
            ok = !subscription_id.empty();
          } else if (status == 409) {
            subscription_id = extract_existing_subscription_id(c->get_content());
            ok = !subscription_id.empty();
          }
          if (!ok) {
            log(ll_error, "Failed to create " + type + " subscription, Helix returned status " + std::to_string(status) + ": " + c->get_content());
          } else if (status == 409) {
            log(ll_info, "Reusing existing " + type + " subscription " + subscription_id);
          }
          defer([this, it]() { pending_app_requests_.erase(it); });
          if (callback) {
            callback(ok, subscription_id);
          }
        });
  });
}

void conduit::unsubscribe(const std::string &subscription_id, std::function<void(bool)> callback) {
  ensure_app_access_token([this, subscription_id, callback](bool ok) {
    if (!ok) {
      log(ll_error, "Failed to delete subscription " + subscription_id + ": could not obtain an app access token");
      if (callback) {
        callback(false);
      }
      return;
    }

    http_headers headers {
        {"Authorization", "Bearer " + app_access_token_},
        {"Client-Id", client_id_},
    };

    auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
    *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, "/helix/eventsub/subscriptions?id=" + utility::url_encode(subscription_id), "DELETE", "",
                                         headers, false, 10, "1.1", [this, it, subscription_id, callback](https_client *c) {
                                           bool ok = (c->get_status() == 204);
                                           if (!ok) {
                                             log(ll_error, "Failed to delete subscription " + subscription_id + ", Helix returned status " +
                                                               std::to_string(c->get_status()) + ": " + c->get_content());
                                           }
                                           defer([this, it]() { pending_app_requests_.erase(it); });
                                           if (callback) {
                                             callback(ok);
                                           }
                                         });
  });
}

void conduit::send_message(consumer *c, const std::string &message, const std::string &broadcaster_id) {
  std::string target = broadcaster_id.empty() ? c->get_user_id() : broadcaster_id;
  json body {
      {"broadcaster_id", target},
      {"sender_id", c->get_user_id()},
      {"message", message},
  };

  helix_post(c, "/helix/chat/messages", body.dump(), [this, login = c->get_login()](https_client *hc) {
    if (hc->get_status() != 200) {
      log(ll_error, "Failed to send chat message as " + login + ", Helix returned status " + std::to_string(hc->get_status()) + ": " + hc->get_content());
    }
  });
}

void conduit::send_message(const user &u, const std::string &message, const std::string &broadcaster_id) {
  auto c = get_consumer(u.id);
  if (!c) {
    log(ll_debug, "send_message: " + u.id + " is not a tracked consumer");
    return;
  }
  send_message(c.get(), message, broadcaster_id);
}

void conduit::helix_post(consumer *c, const std::string &path, const std::string &body, std::function<void(https_client *)> on_done) {
  http_headers headers {
      {"Authorization", "Bearer " + c->access_token_},
      {"Client-Id", client_id_},
      {"Content-Type", "application/json"},
  };

  std::lock_guard<std::mutex> lock(c->pending_requests_mutex_);
  auto it = c->pending_requests_.insert(c->pending_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, path, "POST", body, headers, false, 10, "1.1", [this, c, it, on_done](https_client *hc) {
    if (on_done) {
      on_done(hc);
    }
    /* Erasing `it` here would destroy this https_client mid-callback. */
    defer([c, it]() {
      std::lock_guard<std::mutex> inner_lock(c->pending_requests_mutex_);
      c->pending_requests_.erase(it);
    });
  });
}

void conduit::helix_get(const std::string &path, std::function<void(https_client *)> on_done) {
  ensure_app_access_token([this, path, on_done](bool ok) {
    if (!ok) {
      log(ll_error, "helix_get(" + path + "): could not obtain an app access token");
      if (on_done) {
        on_done(nullptr);
      }
      return;
    }

    http_headers headers {
        {"Authorization", "Bearer " + app_access_token_},
        {"Client-Id", client_id_},
    };

    auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
    *it = std::make_unique<https_client>(this, "api.twitch.tv", 443, path, "GET", "", headers, false, 10, "1.1", [this, it, on_done](https_client *hc) {
      if (on_done) {
        on_done(hc);
      }
      defer([this, it]() { pending_app_requests_.erase(it); });
    });
  });
}

namespace {

std::string helix_query_string(const std::string &key, const std::vector<std::string> &values) {
  std::string query;
  for (const auto &value : values) {
    query += query.empty() ? "?" : "&";
    query += key + "=" + utility::url_encode(value);
  }
  return query;
}

helix_user parse_helix_user(const json &j) {
  helix_user u;
  u.id = j.value("id", "");
  u.login = j.value("login", "");
  u.display_name = j.value("display_name", "");
  u.profile_image_url = j.value("profile_image_url", "");
  return u;
}

helix_stream parse_helix_stream(const json &j) {
  helix_stream s;
  s.id = j.value("id", "");
  s.user_id = j.value("user_id", "");
  s.user_login = j.value("user_login", "");
  s.user_name = j.value("user_name", "");
  s.game_id = j.value("game_id", "");
  s.game_name = j.value("game_name", "");
  s.title = j.value("title", "");
  s.viewer_count = j.value("viewer_count", (int64_t) 0);
  s.started_at = j.value("started_at", "");
  s.thumbnail_url = j.value("thumbnail_url", "");
  return s;
}

}// namespace

void conduit::get_users(const std::vector<std::string> &logins, helix_users_event callback) {
  helix_get("/helix/users" + helix_query_string("login", logins), [callback](https_client *hc) {
    std::vector<helix_user> users;
    bool ok = hc && hc->get_status() == 200;
    if (ok) {
      json resp = json::parse(hc->get_content(), nullptr, false);
      if (!resp.is_discarded() && resp.contains("data") && resp["data"].is_array()) {
        for (const auto &entry : resp["data"]) {
          users.push_back(parse_helix_user(entry));
        }
      } else {
        ok = false;
      }
    }
    if (callback) {
      callback(ok, users);
    }
  });
}

void conduit::get_streams(const std::vector<std::string> &user_logins, helix_streams_event callback) {
  helix_get("/helix/streams" + helix_query_string("user_login", user_logins), [callback](https_client *hc) {
    std::vector<helix_stream> streams;
    bool ok = hc && hc->get_status() == 200;
    if (ok) {
      json resp = json::parse(hc->get_content(), nullptr, false);
      if (!resp.is_discarded() && resp.contains("data") && resp["data"].is_array()) {
        for (const auto &entry : resp["data"]) {
          streams.push_back(parse_helix_stream(entry));
        }
      } else {
        ok = false;
      }
    }
    if (callback) {
      callback(ok, streams);
    }
  });
}

void conduit::set_token_expiry(consumer *c, uint64_t expires_in) {
  c->token_expires_at_ = expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0;
  schedule_token_rotation(c);
}

void conduit::schedule_token_rotation(consumer *c) {
  c->rotation_timer_.reset();

  if (c->refresh_token_.empty() || c->token_expires_at_ == 0) {
    return;
  }

  constexpr time_t safety_margin = 60;
  time_t now = time(nullptr);
  time_t delay = c->token_expires_at_ - now - safety_margin;
  if (delay < 1) {
    delay = 1;
  }

  std::weak_ptr<consumer> self = c->weak_from_this();
  c->rotation_timer_ = std::make_unique<oneshot_timer>(this, static_cast<uint64_t>(delay), [this, self](tpp::timer) {
    if (auto s = self.lock()) {
      refresh_access_token(s.get());
    }
  });
}

void conduit::refresh_access_token(consumer *c) {
  if (c->refresh_token_.empty()) {
    return;
  }

  std::weak_ptr<consumer> self = c->weak_from_this();
  refresh_user_token(c->refresh_token_, [this, self](bool ok, const std::string &access_token, const std::string &refresh_token, uint64_t expires_in) {
    auto s = self.lock();
    if (!s) {
      return;
    }
    if (!ok) {
      log(ll_error, "Failed to rotate access token for " + s->login_);
      return;
    }
    s->access_token_ = access_token;
    if (!refresh_token.empty()) {
      s->refresh_token_ = refresh_token;
    }
    set_token_expiry(s.get(), expires_in);
    log(ll_info, "Rotated access token for " + s->login_);
  });
}

void conduit::get_app_access_token(app_token_event callback) {
  if (client_secret_.empty()) {
    log(ll_error, "get_app_access_token() called without a client_secret");
    if (callback) {
      callback(false, "", 0);
    }
    return;
  }

  std::string body =
      "client_id=" + utility::url_encode(client_id_) + "&client_secret=" + utility::url_encode(client_secret_) + "&grant_type=client_credentials";
  http_headers headers {
      {"Content-Type", "application/x-www-form-urlencoded"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "id.twitch.tv", 443, "/oauth2/token", "POST", body, headers, false, 10, "1.1",
                                       [this, it, callback](https_client *c) {
                                         bool ok = (c->get_status() == 200);
                                         std::string token;
                                         uint64_t expires_in = 0;
                                         if (ok) {
                                           json body_json = json::parse(c->get_content(), nullptr, false);
                                           if (!body_json.is_discarded() && body_json.is_object()) {
                                             token = body_json.value("access_token", "");
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

void conduit::ensure_app_access_token(std::function<void(bool)> callback) {
  if (!app_access_token_.empty()) {
    if (callback) {
      callback(true);
    }
    return;
  }

  get_app_access_token([this, callback](bool ok, const std::string &token, uint64_t expires_in) {
    if (!ok) {
      if (callback) {
        callback(false);
      }
      return;
    }
    app_access_token_ = token;
    app_access_token_expires_at_ = expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0;
    schedule_app_token_rotation();
    if (callback) {
      callback(true);
    }
  });
}

void conduit::schedule_app_token_rotation() {
  app_token_rotation_timer_.reset();

  if (client_secret_.empty() || app_access_token_expires_at_ == 0) {
    return;
  }

  constexpr time_t safety_margin = 60;
  time_t now = time(nullptr);
  time_t delay = app_access_token_expires_at_ - now - safety_margin;
  if (delay < 1) {
    delay = 1;
  }

  app_token_rotation_timer_ = std::make_unique<oneshot_timer>(this, static_cast<uint64_t>(delay), [this](tpp::timer) { refresh_app_token(); });
}

void conduit::refresh_app_token() {
  get_app_access_token([this](bool ok, const std::string &token, uint64_t expires_in) {
    if (!ok) {
      log(ll_error, "Failed to rotate app access token");
      return;
    }
    app_access_token_ = token;
    app_access_token_expires_at_ = expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0;
    schedule_app_token_rotation();
    log(ll_info, "Rotated app access token");
  });
}

void conduit::refresh_user_token(const std::string &refresh_token, refresh_token_event callback) {
  if (client_secret_.empty()) {
    log(ll_error, "refresh_user_token() called without a client_secret");
    if (callback) {
      callback(false, "", "", 0);
    }
    return;
  }

  std::string body = "grant_type=refresh_token&refresh_token=" + utility::url_encode(refresh_token) + "&client_id=" + utility::url_encode(client_id_) +
                     "&client_secret=" + utility::url_encode(client_secret_);
  http_headers headers {
      {"Content-Type", "application/x-www-form-urlencoded"},
  };

  auto it = pending_app_requests_.insert(pending_app_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(this, "id.twitch.tv", 443, "/oauth2/token", "POST", body, headers, false, 10, "1.1",
                                       [this, it, callback](https_client *c) {
                                         bool ok = (c->get_status() == 200);
                                         std::string access_token;
                                         std::string new_refresh_token;
                                         uint64_t expires_in = 0;
                                         if (ok) {
                                           json body_json = json::parse(c->get_content(), nullptr, false);
                                           if (!body_json.is_discarded() && body_json.is_object()) {
                                             access_token = body_json.value("access_token", "");
                                             new_refresh_token = body_json.value("refresh_token", "");
                                             expires_in = body_json.value("expires_in", (uint64_t) 0);
                                           }
                                           ok = !access_token.empty();
                                         }
                                         if (callback) {
                                           callback(ok, access_token, new_refresh_token, expires_in);
                                         }
                                         defer([this, it]() { pending_app_requests_.erase(it); });
                                       });
}

}// namespace tpp
