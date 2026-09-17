#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/https_client.h>
#include <tpp/timer.h>

#include <ctime>
#include <string>
#include <utility>

namespace tpp {

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
  owner_->send_message(this, message, broadcaster_id);
}

void consumer::schedule_token_rotation() {
  owner_->schedule_token_rotation(this);
}

}// namespace tpp
