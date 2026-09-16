#include <tpp/application.h>
#include <tpp/eventsub_client.h>
#include <tpp/https_client.h>
#include <tpp/session.h>
#include <tpp/timer.h>

#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace tpp {

namespace {
using json = nlohmann::json;
}

session::session(application *owner, std::string user_id, std::string login,
                 std::string access_token, std::string id_token,
                 uint64_t expires_in, std::string refresh_token)
    : owner_(owner)
    , user_id_(std::move(user_id))
    , login_(std::move(login))
    , access_token_(std::move(access_token))
    , id_token_(std::move(id_token))
    , refresh_token_(std::move(refresh_token))
    , token_expires_at_(expires_in > 0
                            ? time(nullptr) + static_cast<time_t>(expires_in)
                            : 0) {
}

session::~session() = default;

application &session::get_application() const noexcept {
  return *owner_;
}

const std::string &session::get_client_id() const noexcept {
  return owner_->get_client_id();
}

const std::string &session::get_user_id() const noexcept {
  return user_id_;
}

const std::string &session::get_login() const noexcept {
  return login_;
}

const std::string &session::get_access_token() const noexcept {
  return access_token_;
}

const std::string &session::get_id_token() const noexcept {
  return id_token_;
}

bool session::has_refresh_token() const noexcept {
  return !refresh_token_.empty();
}

const std::string &session::get_refresh_token() const noexcept {
  return refresh_token_;
}

time_t session::get_token_expires_at() const noexcept {
  return token_expires_at_;
}

uint64_t session::get_token_expires_in() const noexcept {
  if (token_expires_at_ == 0) {
    return 0;
  }
  time_t now = time(nullptr);
  return token_expires_at_ > now
             ? static_cast<uint64_t>(token_expires_at_ - now)
             : 0;
}

void session::wire_eventsub_callbacks(eventsub_client *client) {
  std::weak_ptr<session> self = weak_from_this();

  client->on_welcome = [self](const std::string &session_id) {
    if (auto s = self.lock()) {
      s->on_eventsub_welcome(session_id);
    }
  };
  client->on_reconnect = [self](const std::string &reconnect_url) {
    if (auto s = self.lock()) {
      s->on_eventsub_reconnect(reconnect_url);
    }
  };
  client->on_notification = [self](const std::string &subscription_type,
                                   nlohmann::json    &event,
                                   const std::string &event_json) {
    if (auto s = self.lock()) {
      s->handle_notification(subscription_type, event, event_json);
    }
  };
}

void session::connect() {
  eventsub_ = std::make_unique<eventsub_client>(owner_);
  wire_eventsub_callbacks(eventsub_.get());
  schedule_token_rotation();
}

void session::on_eventsub_welcome(const std::string &session_id) {
  eventsub_session_id_ = session_id;

  std::vector<event> pending;
  {
    std::lock_guard<std::mutex> lock(subscribe_mutex_);
    eventsub_ready_ = true;
    pending.swap(pending_subscriptions_);
  }
  for (const auto &e : pending) {
    do_subscribe(e);
  }
}

void session::on_eventsub_reconnect(const std::string &reconnect_url) {
  owner_->log(ll_info, "EventSub session for " + login_ + " reconnecting");
  /* Twitch migrates existing subscriptions to the new session; nothing to
   * resubscribe here. */
  auto new_client = std::make_unique<eventsub_client>(owner_, reconnect_url);
  wire_eventsub_callbacks(new_client.get());

  /* application::defer requires a copy-constructible target. */
  auto shared_client =
      std::make_shared<std::unique_ptr<eventsub_client>>(std::move(new_client));

  /* Replacing eventsub_ here destroys the client whose own call stack
   * invoked this callback, so it must happen on the next event loop tick. */
  std::weak_ptr<session> self = weak_from_this();
  owner_->defer([self, shared_client]() {
    if (auto s = self.lock()) {
      s->eventsub_ = std::move(*shared_client);
    }
  });
}

void session::handle_notification(const std::string &subscription_type,
                                  nlohmann::json    &event,
                                  const std::string &event_json) {
  const events::event_handler *handler =
      events::find_handler(subscription_type);
  if (handler) {
    handler->handle(this, event, event_json);
  }
}

void session::subscribe(const event &e) {
  {
    std::lock_guard<std::mutex> lock(subscribe_mutex_);
    if (!eventsub_ready_) {
      pending_subscriptions_.push_back(e);
      return;
    }
  }
  do_subscribe(e);
}

void session::do_subscribe(const event &e) {
  json condition = json::object();
  for (const auto &[key, value] : e.condition) {
    condition[key] = value;
  }

  json body {
      {"type",      e.type                                            },
      {"version",   e.version                                         },
      {"condition", condition                                         },
      {"transport",
       {{"method", "websocket"}, {"session_id", eventsub_session_id_}}},
  };

  helix_post("/helix/eventsub/subscriptions", body.dump(),
             [this, type = e.type](https_client *c) {
               if (c->get_status() != 202) {
                 owner_->log(ll_error, "Failed to create " + type +
                                           " subscription for " + login_ +
                                           ", Helix returned status " +
                                           std::to_string(c->get_status()) +
                                           ": " + c->get_content());
               }
             });
}

void session::send_message(const std::string &message,
                           const std::string &broadcaster_id) {
  std::string target = broadcaster_id.empty() ? user_id_ : broadcaster_id;
  json        body {
             {"broadcaster_id", target  },
             {"sender_id",      user_id_},
             {"message",        message },
  };

  helix_post("/helix/chat/messages", body.dump(), [this](https_client *c) {
    if (c->get_status() != 200) {
      owner_->log(ll_error, "Failed to send chat message as " + login_ +
                                ", Helix returned status " +
                                std::to_string(c->get_status()) + ": " +
                                c->get_content());
    }
  });
}

void session::set_token_expiry(uint64_t expires_in) {
  token_expires_at_ =
      expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0;
  schedule_token_rotation();
}

void session::schedule_token_rotation() {
  rotation_timer_.reset();

  if (refresh_token_.empty() || token_expires_at_ == 0) {
    return;
  }

  constexpr time_t safety_margin = 60;
  time_t           now           = time(nullptr);
  time_t           delay         = token_expires_at_ - now - safety_margin;
  if (delay < 1) {
    delay = 1;
  }

  std::weak_ptr<session> self = weak_from_this();
  rotation_timer_             = std::make_unique<oneshot_timer>(
      owner_, static_cast<uint64_t>(delay), [self](tpp::timer) {
        if (auto s = self.lock()) {
          s->refresh_access_token();
        }
      });
}

void session::refresh_access_token() {
  if (refresh_token_.empty()) {
    return;
  }

  std::weak_ptr<session> self = weak_from_this();
  owner_->refresh_user_token(
      refresh_token_,
      [self](bool ok, const std::string &access_token,
             const std::string &refresh_token, uint64_t expires_in) {
        auto s = self.lock();
        if (!s) {
          return;
        }
        if (!ok) {
          s->owner_->log(ll_error,
                         "Failed to rotate access token for " + s->login_);
          return;
        }
        s->access_token_ = access_token;
        if (!refresh_token.empty()) {
          s->refresh_token_ = refresh_token;
        }
        s->set_token_expiry(expires_in);
        s->owner_->log(ll_info, "Rotated access token for " + s->login_);
      });
}

void session::helix_post(const std::string &path, const std::string &body,
                         std::function<void(https_client *)> on_done) {
  http_headers headers {
      {"Authorization", "Bearer " + access_token_},
      {"Client-Id",     get_client_id()          },
      {"Content-Type",  "application/json"       },
  };

  std::lock_guard<std::mutex> lock(pending_requests_mutex_);
  auto it = pending_requests_.insert(pending_requests_.end(), nullptr);
  *it     = std::make_unique<https_client>(
      owner_, "api.twitch.tv", 443, path, "POST", body, headers, false, 10,
      "1.1", [this, it, on_done](https_client *c) {
        if (on_done) {
          on_done(c);
        }
        /* Erasing `it` here would destroy this https_client mid-callback. */
        owner_->defer([this, it]() {
          std::lock_guard<std::mutex> inner_lock(pending_requests_mutex_);
          pending_requests_.erase(it);
        });
      });
}

}// namespace tpp
