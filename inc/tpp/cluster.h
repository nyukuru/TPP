#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "tpp/export.h"

namespace tpp {

enum class intents : uint64_t {
  none           = 0,
  chat_messages  = 1 << 0,
  channel_points = 1 << 1,
  subscriptions  = 1 << 2,
  follows        = 1 << 3,
  raids          = 1 << 4,
  all = chat_messages | channel_points | subscriptions | follows | raids
};

inline intents operator|(intents a, intents b) {
  return static_cast<intents>(static_cast<uint64_t>(a) |
                              static_cast<uint64_t>(b));
}

inline intents operator&(intents a, intents b) {
  return static_cast<intents>(static_cast<uint64_t>(a) &
                              static_cast<uint64_t>(b));
}

class TPP_EXPORT cluster {
 private:
  std::string                  token_;
  intents                      intents_;
  std::atomic<bool>            running_ {false};
  std::unique_ptr<std::thread> worker_thread_;

  void worker_loop();

 public:
  explicit cluster(const std::string& token,
                   intents            intent_flags = intents::chat_messages);

  ~cluster();

  cluster(const cluster&)            = delete;
  cluster& operator=(const cluster&) = delete;
  cluster(cluster&&)                 = delete;
  cluster& operator=(cluster&&)      = delete;

  void start(bool wait = true);

  void stop();

  bool is_running() const noexcept;

  const std::string& get_token() const noexcept;

  intents get_intents() const noexcept;

  void join_channel(const std::string& channel_name);

  void leave_channel(const std::string& channel_name);

  void send_message(const std::string& channel_name,
                    const std::string& message);
};

}// namespace tpp