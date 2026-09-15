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

#include <tpp/dns.h>

#include <cstring>
#include <mutex>
#include <shared_mutex>

namespace tpp {
constexpr time_t one_hour = 60 * 60;

std::shared_mutex dns_cache_mutex;
dns_cache_t       dns_cache;

/**
 * @brief Get address length
 * @return address length
 */
int dns_cache_entry::size() const {
  return static_cast<int>(addr.ai_addrlen);
}

const address_t dns_cache_entry::get_connecting_address(uint16_t port) const {
  return address_t(resolved_addr, port);
}

socket dns_cache_entry::make_connecting_socket() const {
  return ::socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol);
}

const dns_cache_entry *resolve_hostname(const std::string &hostname,
                                        const std::string &port) {
  addrinfo                    hints, *addrs;
  dns_cache_t::const_iterator iter;
  time_t                      now = time(nullptr);
  int                         error;
  bool                        exists = false;

  {
    std::shared_lock dns_cache_lock(dns_cache_mutex);
    iter = dns_cache.find(hostname);
    if (iter != dns_cache.end()) {
      exists = true;
      if (now < iter->second->expire_timestamp) {
        return iter->second.get();
      }
    }
  }
  if (exists) {
    std::unique_lock dns_cache_lock(dns_cache_mutex);
    iter = dns_cache.find(hostname);
    if (iter != dns_cache.end()) { /* re-validate iter */
      dns_cache.erase(iter);
    }
  }

  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if ((error = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &addrs))) {
    throw std::string("getaddrinfo error: ") + gai_strerror(error);
  }

  {
    std::unique_lock dns_cache_lock(dns_cache_mutex);
    auto             cache_entry = std::make_unique<dns_cache_entry>();

    for (struct addrinfo *rp = addrs; rp != nullptr; rp = rp->ai_next) {
      if (rp->ai_family != AF_INET) {
        continue;
      }
      memcpy(&cache_entry->addr, rp, sizeof(addrinfo));
      char        buffer[128];
      sockaddr_in in {};
      std::memcpy(&in, rp->ai_addr, sizeof(sockaddr_in));
      if (inet_ntop(rp->ai_family, &in.sin_addr, buffer, sizeof(buffer))) {
        cache_entry->resolved_addr = buffer;
      }
      break;
    }

    cache_entry->expire_timestamp = now + one_hour;
    auto r = dns_cache.emplace(hostname, std::move(cache_entry));

    freeaddrinfo(addrs);
    return r.first->second.get();
  }
}

}// namespace tpp
