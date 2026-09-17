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

#include <tpp/conduit.h>
#include <tpp/http_server_request.h>
#include <tpp/socket_listener.h>
#include <tpp/ssl_context.h>

namespace tpp {

/**
 * @brief Creates a simple HTTP server which listens on a TCP port for a
 * plaintext or SSL incoming request, and passes each request to a callback
 * to generate the response.
 */
struct TPP_EXPORT http_server : public socket_listener<http_server_request> {
  /**
   * @brief Request handler callback used for all incoming HTTP(S) requests
   */
  http_server_request_event request_handler;

  /**
   * @brief Port we are listening on
   */
  uint16_t bound_port;

  /**
   * @brief Creates a HTTP(S) server.
   * @param creator owning conduit
   * @param address address to bind to; "0.0.0.0" binds all local
   * addresses
   * @param port port to bind to
   * @param handle_request callback to call for each pending request
   * @param private_key private key PEM file for HTTPS/SSL. If empty, a
   * plaintext server is created
   * @param public_key public key PEM file for HTTPS/SSL. If empty, a
   * plaintext server is created
   */
  http_server(conduit *creator, const std::string_view address, uint16_t port, http_server_request_event handle_request, const std::string &private_key = "",
              const std::string &public_key = "");

  /**
   * @brief Emplaces a new request into the connection pool.
   * @param newfd file descriptor of the new request
   */
  void emplace(socket newfd) override;

  virtual ~http_server() {
    detail::release_ssl_context(bound_port);
  }
};

}// namespace tpp
