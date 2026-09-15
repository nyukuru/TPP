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
#include <tpp/application.h>
#include <tpp/exception.h>
#include <tpp/socketengine.h>

#ifndef _WIN32
#  include <csignal>
#endif
#include <ctime>
#include <memory>

namespace tpp {

#ifndef _WIN32
void set_signal_handler(int signal) {
  struct sigaction sa {};
  sigaction(signal, nullptr, &sa);
  if (sa.sa_flags == 0 && sa.sa_handler == nullptr) {
    sa = {};
    sigaction(signal, &sa, nullptr);
  }
}
#endif

bool socket_engine_base::register_socket(const socket_events &e) {
  std::unique_lock lock(fds_mutex);
  auto             i = fds.find(e.fd);
  if (e.fd != INVALID_SOCKET && i == fds.end()) {
    fds.emplace(e.fd, std::make_unique<socket_events>(e));
    stats.active_fds++;
    return true;
  }
  if (e.fd != INVALID_SOCKET && i != fds.end()) {
    remove_socket(e.fd);
    fds.erase(i);
    fds.emplace(e.fd, std::make_unique<socket_events>(e));
    stats.updates++;
    return true;
  }
  return false;
}

bool socket_engine_base::update_socket(const socket_events &e) {
  std::unique_lock lock(fds_mutex);
  auto             iter = fds.find(e.fd);
  if (e.fd != INVALID_SOCKET && iter != fds.end()) {
    *(iter->second) = e;
    stats.updates++;
    return true;
  }
  return false;
}

socket_engine_base::socket_engine_base(application *creator) : owner(creator) {
#ifndef _WIN32
  set_signal_handler(SIGCHLD);
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
#else
  WSADATA wsadata;
  if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
    throw tpp::connection_exception(err_connect_failure, "WSAStartup failure");
  }
#endif
}

socket_engine_base::~socket_engine_base() {
#ifdef _WIN32
  WSACleanup();
#endif
}

namespace {
time_t last_time = time(nullptr);
}

socket_events *socket_engine_base::get_fd(tpp::socket fd) {
  std::unique_lock lock(fds_mutex);
  auto             iter = fds.find(fd);
  if (iter == fds.end()) {
    return nullptr;
  }
  return iter->second.get();
}

void socket_engine_base::inplace_modify_fd(tpp::socket fd,
                                           uint8_t     extra_flags) {
  bool          should_modify {false};
  socket_events s {};
  {
    std::lock_guard<std::shared_mutex> lock(fds_mutex);
    auto                               i = fds.find(fd);
    should_modify =
        i != fds.end() && (i->second->flags & extra_flags) != extra_flags;
    if (should_modify) {
      i->second->flags |= extra_flags;
      s = *(i->second);
    }
  }
  if (should_modify) {
    update_socket(s);
  }
}

void socket_engine_base::prune() {
  if (time(nullptr) != last_time) {
    try {
      owner->tick_timers();
    } catch (const std::exception &e) {
      owner->log(tpp::ll_error,
                 "Uncaught exception in tick_timers: " + std::string(e.what()));
    }
    last_time = time(nullptr);
  }
  stats.iterations++;
}

bool socket_engine_base::delete_socket(tpp::socket fd) {
  std::unique_lock lock(fds_mutex);
  auto             iter = fds.find(fd);
  if (iter == fds.end() || ((iter->second->flags & WANT_DELETION) != 0)) {
    return false;
  }
  iter->second->flags &= ~(WANT_WRITE | WANT_READ | WANT_ERROR);
  iter->second->flags |= WANT_DELETION;
  stats.deletions++;
  stats.active_fds--;
  return true;
}

bool socket_engine_base::remove_socket(tpp::socket) {
  return true;
}

const socket_stats &socket_engine_base::get_stats() const {
  return stats;
}

}// namespace tpp
