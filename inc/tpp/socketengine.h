/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2021 Craig Edwards and D++ contributors
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#pragma once
#include <tpp/export.h>
#include <tpp/socket.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tpp {

class application;

/**
 * @brief Types of IO events a socket may subscribe to.
 */
enum socket_event_flags : uint8_t {
  /**
   * @brief Socket wants to receive events when it can be read from.
   */
  WANT_READ = 1,
  /**
   * @brief Socket wants to receive events when it can be written to. This is
   * a one-off event: to receive further write events, request it again.
   */
  WANT_WRITE = 2,
  /**
   * @brief Socket wants to receive events that indicate an error condition.
   */
  WANT_ERROR = 4,
  /**
   * @brief Socket should be removed as soon as is safe to do so.
   */
  WANT_DELETION = 8,
};

/**
 * @brief Read ready event
 */
using socket_read_event =
    std::function<void(tpp::socket fd, const struct socket_events &)>;

/**
 * @brief Write ready event
 */
using socket_write_event =
    std::function<void(tpp::socket fd, const struct socket_events &)>;

/**
 * @brief Error event
 */
using socket_error_event = std::function<void(
    tpp::socket fd, const struct socket_events &, int error_code)>;

/**
 * @brief Contains statistics about the IO loop
 */
struct TPP_EXPORT socket_stats {
  uint64_t         reads {0};
  uint64_t         writes {0};
  uint64_t         errors {0};
  uint64_t         updates {0};
  uint64_t         deletions {0};
  uint64_t         iterations {0};
  uint64_t         active_fds {0};
  std::string_view engine_type;
};

/**
 * @brief Represents an active socket event set in the socket engine.
 *
 * An event set contains a file descriptor, a set of event handler callbacks,
 * and a set of bitmask flags which indicate which events it wants to
 * receive.
 */
struct TPP_EXPORT socket_events {
  /**
   * @brief File descriptor
   */
  tpp::socket fd {INVALID_SOCKET};

  /**
   * @brief Flag bit mask of values from tpp::socket_event_flags
   */
  uint8_t flags {0};

  /**
   * @brief Read ready event
   */
  socket_read_event on_read {};

  /**
   * @brief Write ready event
   */
  socket_write_event on_write {};

  /**
   * @brief Error event
   */
  socket_error_event on_error {};

  socket_events(tpp::socket socket_fd, uint8_t _flags,
                const socket_read_event  &read_event,
                const socket_write_event &write_event = {},
                const socket_error_event &error_event = {})
      : fd(socket_fd)
      , flags(_flags)
      , on_read(read_event)
      , on_write(write_event)
      , on_error(error_event) {
  }

  socket_events() = default;
};

/**
 * @brief Container of event sets keyed by socket file descriptor
 */
using socket_container =
    std::unordered_map<tpp::socket, std::unique_ptr<socket_events>>;

/**
 * @brief Base class for socket engines. The implementation drives IO via
 * ::poll().
 */
struct TPP_EXPORT socket_engine_base {
  /**
   * @brief Owning application
   */
  class application *owner {nullptr};

  explicit socket_engine_base(class application *creator);

  socket_engine_base(const socket_engine_base &)            = delete;
  socket_engine_base(socket_engine_base &&)                 = delete;
  socket_engine_base &operator=(const socket_engine_base &) = delete;
  socket_engine_base &operator=(socket_engine_base &&)      = delete;

  virtual ~socket_engine_base();

  /**
   * @brief Should be called repeatedly in a loop. Will run for a maximum of
   * around 1 second.
   */
  virtual void process_events() = 0;

  /**
   * @brief Register a new socket with the socket engine
   * @param e Socket events
   * @return true if socket was added
   */
  virtual bool register_socket(const socket_events &e);

  /**
   * @brief Update an existing socket in the socket engine
   * @param e Socket events
   * @return true if socket was updated
   */
  virtual bool update_socket(const socket_events &e);

  /**
   * @brief Queue a socket for deletion from the socket engine. The socket is
   * not removed immediately, but marked WANT_DELETION so it is removed as
   * soon as is safe to do so.
   * @param fd File descriptor
   * @return true if socket was queued for deletion
   */
  bool delete_socket(tpp::socket fd);

  /**
   * @brief Iterate through the list of sockets and remove any with
   * WANT_DELETION set.
   */
  void prune();

  /**
   * @brief Merge new flags in with the given file descriptor
   * @param fd file descriptor
   * @param extra_flags extra flags to add
   */
  void inplace_modify_fd(tpp::socket fd, uint8_t extra_flags);

  /**
   * @brief Get statistics for socket engine
   * @return socket stats
   */
  const socket_stats &get_stats() const;

 protected:
  /**
   * @brief Mutex for fds
   */
  std::shared_mutex fds_mutex;

  /**
   * @brief File descriptors, and their states
   */
  socket_container fds;

  /**
   * @brief Socket engine statistics
   */
  socket_stats stats {};

  /**
   * @brief Find a file descriptor's socket events
   * @param fd file descriptor
   * @return file descriptor or nullptr if doesn't exist
   */
  socket_events *get_fd(tpp::socket fd);

  /**
   * @brief Called by prune() to remove sockets when safe to do so.
   * @param fd File descriptor to remove
   */
  virtual bool remove_socket(tpp::socket fd);
};

/**
 * @brief Creates the socket engine implementation.
 * @param creator Creating application
 */
TPP_EXPORT std::unique_ptr<socket_engine_base> create_socket_engine(
    class application *creator);

/**
 * @brief Fired when a socket managed by the socket engine is closed.
 */
struct TPP_EXPORT socket_close_t {
  /**
   * @brief File descriptor that was closed
   */
  tpp::socket fd {INVALID_SOCKET};
};

/**
 * @brief A minimal multi-subscriber event signal, used for internal
 * lifecycle notifications such as tpp::application::on_socket_close.
 * @tparam T event payload type
 */
template<typename T>
class event_router_t {
  std::unordered_map<size_t, std::function<void(const T &)>> handlers;
  size_t                                                     next_handle {1};
  mutable std::mutex                                         guard;

 public:
  /**
   * @brief Attach a new handler to this event
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
   * @brief Detach a previously attached handler
   * @param handle handle returned from operator()
   */
  void detach(size_t handle) {
    std::lock_guard<std::mutex> lock(guard);
    handlers.erase(handle);
  }

  /**
   * @brief Call all attached handlers with the given event
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
   * @brief True if there are no handlers attached
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
