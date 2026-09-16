#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/https_client.h>
#include <tpp/timer.h>

#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace tpp {

namespace {
using json = nlohmann::json;
}

consumer::consumer(conduit *owner, std::string user_id, std::string login, std::string access_token, std::string id_token, uint64_t expires_in,
                   std::string refresh_token)
    : owner_(owner)
    , user_id_(std::move(user_id))
    , login_(std::move(login))
    , access_token_(std::move(access_token))
    , id_token_(std::move(id_token))
    , refresh_token_(std::move(refresh_token))
    , token_expires_at_(expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0) {
}

consumer::~consumer() = default;

conduit &consumer::get_conduit() const noexcept {
  return *owner_;
}

const std::string &consumer::get_client_id() const noexcept {
  return owner_->get_client_id();
}

const std::string &consumer::get_user_id() const noexcept {
  return user_id_;
}

const std::string &consumer::get_login() const noexcept {
  return login_;
}

const std::string &consumer::get_access_token() const noexcept {
  return access_token_;
}

const std::string &consumer::get_id_token() const noexcept {
  return id_token_;
}

bool consumer::has_refresh_token() const noexcept {
  return !refresh_token_.empty();
}

const std::string &consumer::get_refresh_token() const noexcept {
  return refresh_token_;
}

time_t consumer::get_token_expires_at() const noexcept {
  return token_expires_at_;
}

uint64_t consumer::get_token_expires_in() const noexcept {
  if (token_expires_at_ == 0) {
    return 0;
  }
  time_t now = time(nullptr);
  return token_expires_at_ > now ? static_cast<uint64_t>(token_expires_at_ - now) : 0;
}

void consumer::subscribe(const event &e) {
  owner_->subscribe_for_consumer(shared_from_this(), e);
}

void consumer::send_message(const std::string &message, const std::string &broadcaster_id) {
  std::string target = broadcaster_id.empty() ? user_id_ : broadcaster_id;
  json body {
      {"broadcaster_id", target},
      {"sender_id", user_id_},
      {"message", message},
  };

  helix_post("/helix/chat/messages", body.dump(), [this](https_client *c) {
    if (c->get_status() != 200) {
      owner_->log(ll_error,
                  "Failed to send chat message as " + login_ + ", Helix returned status " + std::to_string(c->get_status()) + ": " + c->get_content());
    }
  });
}

void consumer::set_token_expiry(uint64_t expires_in) {
  token_expires_at_ = expires_in > 0 ? time(nullptr) + static_cast<time_t>(expires_in) : 0;
  schedule_token_rotation();
}

void consumer::schedule_token_rotation() {
  rotation_timer_.reset();

  if (refresh_token_.empty() || token_expires_at_ == 0) {
    return;
  }

  constexpr time_t safety_margin = 60;
  time_t now = time(nullptr);
  time_t delay = token_expires_at_ - now - safety_margin;
  if (delay < 1) {
    delay = 1;
  }

  std::weak_ptr<consumer> self = weak_from_this();
  rotation_timer_ = std::make_unique<oneshot_timer>(owner_, static_cast<uint64_t>(delay), [self](tpp::timer) {
    if (auto s = self.lock()) {
      s->refresh_access_token();
    }
  });
}

void consumer::refresh_access_token() {
  if (refresh_token_.empty()) {
    return;
  }

  std::weak_ptr<consumer> self = weak_from_this();
  owner_->refresh_user_token(refresh_token_, [self](bool ok, const std::string &access_token, const std::string &refresh_token, uint64_t expires_in) {
    auto s = self.lock();
    if (!s) {
      return;
    }
    if (!ok) {
      s->owner_->log(ll_error, "Failed to rotate access token for " + s->login_);
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

void consumer::helix_post(const std::string &path, const std::string &body, std::function<void(https_client *)> on_done) {
  http_headers headers {
      {"Authorization", "Bearer " + access_token_},
      {"Client-Id", get_client_id()},
      {"Content-Type", "application/json"},
  };

  std::lock_guard<std::mutex> lock(pending_requests_mutex_);
  auto it = pending_requests_.insert(pending_requests_.end(), nullptr);
  *it = std::make_unique<https_client>(owner_, "api.twitch.tv", 443, path, "POST", body, headers, false, 10, "1.1", [this, it, on_done](https_client *c) {
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
