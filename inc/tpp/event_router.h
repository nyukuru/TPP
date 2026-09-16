#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace tpp {

/**
 * @brief A multi-subscriber event signal.
 * @tparam T event payload type
 */
template<typename T>
class event_router_t {
  std::unordered_map<size_t, std::function<void(const T &)>> handlers;
  size_t                                                     next_handle {1};
  mutable std::mutex                                         guard;

 public:
  /**
   * @brief Attaches a new handler to this event.
   * @param handler handler to call when the event fires
   * @return handle usable with detach()
   */
  size_t operator()(std::function<void(const T &)> handler) {
    std::lock_guard<std::mutex> lock(guard);
    size_t                      handle = next_handle++;
    handlers.emplace(handle, std::move(handler));
    return handle;
  }

  /**
   * @brief Detaches a previously attached handler.
   * @param handle handle returned from operator()
   */
  void detach(size_t handle) {
    std::lock_guard<std::mutex> lock(guard);
    handlers.erase(handle);
  }

  /**
   * @brief Calls every attached handler with the given event.
   * @param event event payload
   */
  void call(const T &event) const {
    std::vector<std::function<void(const T &)>> copy;
    {
      std::lock_guard<std::mutex> lock(guard);
      copy.reserve(handlers.size());
      for (const auto &[handle, handler] : handlers) {
        copy.push_back(handler);
      }
    }
    for (const auto &handler : copy) {
      handler(event);
    }
  }

  /**
   * @brief True if there are no handlers attached.
   */
  [[nodiscard]] bool empty() const {
    std::lock_guard<std::mutex> lock(guard);
    return handlers.empty();
  }
};

/**
 * @brief Handle returned from event_router_t::operator(), used with
 * detach().
 */
using event_handle = size_t;

}// namespace tpp
