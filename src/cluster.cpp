#include "tpp/cluster.h"

#include <chrono>
#include <stdexcept>
#include <thread>

namespace tpp {

cluster::cluster(const std::string& token, intents intent_flags)
    : token_(token), intents_(intent_flags) {
  if (token.empty()) {
    throw std::invalid_argument("Token cannot be empty");
  }
}

cluster::~cluster() {
  if (running_.load()) {
    stop();
  }
}

void cluster::start(bool wait) {
  if (running_.load()) {
    throw std::runtime_error("Cluster is already running");
  }

  running_.store(true);
  worker_thread_ = std::make_unique<std::thread>(&cluster::worker_loop, this);

  if (wait && worker_thread_ && worker_thread_->joinable()) {
    worker_thread_->join();
  }
}

void cluster::stop() {
  if (!running_.load()) {
    return;
  }

  running_.store(false);

  if (worker_thread_ && worker_thread_->joinable()) {
    worker_thread_->join();
  }

  worker_thread_.reset();
}

bool cluster::is_running() const noexcept {
  return running_.load();
}

const std::string& cluster::get_token() const noexcept {
  return token_;
}

intents cluster::get_intents() const noexcept {
  return intents_;
}

void cluster::worker_loop() {
  while (running_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void cluster::join_channel(const std::string& channel_name) {
  if (channel_name.empty()) {
    throw std::invalid_argument("Channel name cannot be empty");
  }
  // TODO: Implement Twitch IRC connection and JOIN command
}

void cluster::leave_channel(const std::string& channel_name) {
  if (channel_name.empty()) {
    throw std::invalid_argument("Channel name cannot be empty");
  }
  // TODO: Implement Twitch IRC PART command
}

void cluster::send_message(const std::string& channel_name,
                           const std::string& message) {
  if (channel_name.empty()) {
    throw std::invalid_argument("Channel name cannot be empty");
  }
  if (message.empty()) {
    throw std::invalid_argument("Message cannot be empty");
  }
  // TODO: Implement Twitch IRC PRIVMSG command
}

}// namespace tpp