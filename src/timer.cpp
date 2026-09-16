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
#include <tpp/conduit.h>
#include <tpp/timer.h>

#include <atomic>
#include <ctime>

namespace tpp {

namespace {
std::atomic<timer> next_handle {1};
}

timer conduit::start_timer(timer_callback_t on_tick, uint64_t frequency, timer_callback_t on_stop) {
  timer_t new_timer;

  new_timer.handle = next_handle++;
  new_timer.next_tick = time(nullptr) + static_cast<time_t>(frequency);
  new_timer.on_tick = std::move(on_tick);
  new_timer.on_stop = std::move(on_stop);
  new_timer.frequency = frequency;

  std::lock_guard<std::mutex> lock(timer_guard);
  next_timer.emplace(new_timer);

  return new_timer.handle;
}

bool conduit::stop_timer(timer t) {
  /* Marks the handle deleted; tick_timers() checks this set rather than
   * removing entries from the priority queue directly. */
  std::lock_guard<std::mutex> lock(timer_guard);
  deleted_timers.emplace(t);
  return true;
}

void conduit::tick_timers() {
  time_t now = time(nullptr);

  /* Bounded to visit each timer that existed at the start at most once,
   * so a zero-frequency timer can't loop forever. */
  size_t iterations;
  {
    std::lock_guard<std::mutex> lock(timer_guard);
    if (next_timer.empty()) {
      return;
    }
    iterations = next_timer.size();
  }

  for (size_t i = 0; i < iterations; ++i) {
    timer_t cur_timer;
    {
      std::lock_guard<std::mutex> lock(timer_guard);
      if (next_timer.empty() || next_timer.top().next_tick > now) {
        break;
      }
      cur_timer = next_timer.top();
      next_timer.pop();
    }

    bool deleted {false};
    timers_deleted_t::iterator deleted_iter {};
    {
      std::lock_guard<std::mutex> lock(timer_guard);
      deleted_iter = deleted_timers.find(cur_timer.handle);
      deleted = deleted_iter != deleted_timers.end();
    }

    if (!deleted) {
      if (cur_timer.on_tick) {
        cur_timer.on_tick(cur_timer.handle);
      }
      cur_timer.next_tick += static_cast<time_t>(cur_timer.frequency);
      std::lock_guard<std::mutex> lock(timer_guard);
      next_timer.emplace(std::move(cur_timer));
    } else {
      if (cur_timer.on_stop) {
        cur_timer.on_stop(cur_timer.handle);
      }
      std::lock_guard<std::mutex> lock(timer_guard);
      deleted_timers.erase(deleted_iter);
    }
  }
}

oneshot_timer::oneshot_timer(class conduit *cl, uint64_t duration, timer_callback_t callback) : owner(cl) {
  th = cl->start_timer(
      [callback, this](tpp::timer handle) {
        callback(handle);
        this->owner->stop_timer(this->th);
      },
      duration);
}

timer oneshot_timer::get_handle() {
  return th;
}

void oneshot_timer::cancel() {
  owner->stop_timer(th);
}

oneshot_timer::~oneshot_timer() {
  cancel();
}

}// namespace tpp
