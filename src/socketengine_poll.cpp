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
#include <tpp/compat.h>
#include <tpp/conduit.h>
#include <tpp/exception.h>
#include <tpp/socket.h>
#include <tpp/socketengine.h>
#include <tpp/ssl_connection.h>

#ifndef _WIN32
#  include <sys/socket.h>
#endif

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

#ifndef FD_SETSIZE
#  define FD_SETSIZE 1024
#endif

namespace tpp {

/**
 * @brief A socket engine which multiplexes IO via ::poll().
 */
struct TPP_EXPORT socket_engine_poll : public socket_engine_base {
  std::vector<tpp::compat::pollfd> poll_set;
  tpp::compat::pollfd out_set[FD_SETSIZE] {};
  std::shared_mutex poll_set_mutex;

  void process_events() final {
    constexpr int poll_delay = 1000;

    prune();

    size_t fd_count = 0;
    {
      std::shared_lock lock(poll_set_mutex);
      if (poll_set.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return;
      }
      if (poll_set.size() > FD_SETSIZE) {
        throw tpp::connection_exception(
            "poll() does not support more than FD_SETSIZE active sockets at "
            "once!");
      }
      fd_count = poll_set.size();
      std::copy(poll_set.begin(), poll_set.end(), out_set);
    }

    int i = tpp::compat::poll(out_set, (unsigned int) fd_count, poll_delay);
    int processed = 0;

    for (size_t index = 0; index < fd_count && processed < i; index++) {
      const tpp::socket fd = out_set[index].fd;
      const short revents = out_set[index].revents;

      if (revents > 0) {
        processed++;
      }

      if (fd == wake_read.fd) {
        if ((revents & POLLIN) != 0) {
          drain_wakeup_socket();
        }
        continue;
      }

      socket_events *eh = get_fd(fd);
      if (eh == nullptr) {
        continue;
      }

      if ((eh->flags & WANT_DELETION) == 0) {
        try {
          if ((revents & POLLHUP) != 0) {
            eh->on_error(fd, *eh, 0);
            stats.errors++;
            continue;
          }

          if ((revents & POLLERR) != 0) {
            socklen_t codesize = sizeof(int);
            int errcode {};
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *) &errcode, &codesize) < 0) {
              errcode = errno;
            }
            stats.errors++;
            eh->on_error(fd, *eh, errcode);
            continue;
          }

          if ((revents & POLLIN) != 0) {
            stats.reads++;
            eh->on_read(fd, *eh);
          }

          if ((revents & POLLOUT) != 0) {
            stats.writes++;
            eh->flags &= ~WANT_WRITE;
            update_socket(*eh);
            eh->on_write(fd, *eh);
          }
        } catch (const std::exception &) {
          stats.errors++;
          eh->on_error(fd, *eh, 0);
        }
      }

      if ((eh->flags & WANT_DELETION) != 0) {
        remove_socket(fd);
        std::unique_lock lock(fds_mutex);
        fds.erase(fd);
      }
    }
  }

  bool register_socket(const socket_events &e) final {
    bool r = socket_engine_base::register_socket(e);
    if (r) {
      std::unique_lock lock(poll_set_mutex);
      tpp::compat::pollfd fd_info {};
      fd_info.fd = e.fd;
      fd_info.events = 0;
      if ((e.flags & WANT_READ) != 0) {
        fd_info.events |= POLLIN;
      }
      if ((e.flags & WANT_WRITE) != 0) {
        fd_info.events |= POLLOUT;
      }
      poll_set.push_back(fd_info);
    }
    force_poll_update();
    return r;
  }

  bool update_socket(const socket_events &e) final {
    bool r = socket_engine_base::update_socket(e);
    if (r) {
      std::unique_lock lock(poll_set_mutex);
      for (auto &fd_info : poll_set) {
        if (fd_info.fd != e.fd) {
          continue;
        }
        fd_info.events = 0;
        if ((e.flags & WANT_READ) != 0) {
          fd_info.events |= POLLIN;
        }
        if ((e.flags & WANT_WRITE) != 0) {
          fd_info.events |= POLLOUT;
        }
        break;
      }
    }
    return r;
  }

  explicit socket_engine_poll(conduit *creator) : socket_engine_base(creator) {
    stats.engine_type = "poll";
    init_wakeup_socket();
  }

 protected:
  /* Loopback socket pair used to wake poll() early when a new socket is
   * registered, rather than waiting for the poll timeout. */
  tpp::raii_socket wake_read {tpp::rst_udp};
  tpp::raii_socket wake_write {tpp::rst_udp};

  bool remove_socket(tpp::socket fd) final {
    std::unique_lock lock(poll_set_mutex);
    for (auto i = poll_set.begin(); i != poll_set.end(); ++i) {
      if (i->fd == fd) {
        poll_set.erase(i);
        if (!owner->on_socket_close.empty()) {
          socket_close_t event;
          event.fd = fd;
          owner->on_socket_close.call(event);
        }
        return true;
      }
    }
    return false;
  }

  void init_wakeup_socket() {
    if (!wake_read.bind(tpp::address_t("127.0.0.1", 0))) {
      throw tpp::connection_exception("Failed to bind reading socket of poll wakeup pair");
    }
    if (!set_nonblocking(wake_read.fd, true)) {
      throw tpp::connection_exception(
          "Failed to set reading socket of poll wakeup pair to "
          "non-blocking mode");
    }

    tpp::address_t tmp;
    uint16_t port = tmp.get_port(wake_read.fd);
    tpp::address_t dest("127.0.0.1", port);
    if (::connect(wake_write.fd, dest.get_socket_address(), (int) dest.size()) != 0) {
      throw tpp::connection_exception("Failed to connect writing socket of poll wakeup pair");
    }

    std::unique_lock lock(poll_set_mutex);
    tpp::compat::pollfd fd_info {};
    fd_info.fd = wake_read.fd;
    fd_info.events = POLLIN;
    poll_set.push_back(fd_info);
  }

  void drain_wakeup_socket() {
    char buf[256];
    while (true) {
#ifdef _WIN32
      int r = ::recv(wake_read.fd, buf, sizeof(buf), 0);
      if (r <= 0) {
        break;
      }
#else
      ssize_t r = ::recv(wake_read.fd, buf, sizeof(buf), MSG_DONTWAIT);
      if (r <= 0) {
        break;
      }
#endif
    }
  }

  void force_poll_update() const {
    if (wake_write.fd == INVALID_SOCKET) {
      return;
    }
    static const char one = 1;
    (void) ::send(wake_write.fd, &one, 1, 0);
  }
};

TPP_EXPORT std::unique_ptr<socket_engine_base> create_socket_engine(conduit *creator) {
  return std::make_unique<socket_engine_poll>(creator);
}

}// namespace tpp
