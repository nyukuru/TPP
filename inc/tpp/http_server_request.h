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
#include <tpp/https_client.h>
#include <tpp/ssl_connection.h>

#include <climits>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace tpp {

/**
 * @brief Callback type for HTTP server request callbacks
 */
using http_server_request_event =
    std::function<void(class http_server_request *)>;

/**
 * @brief A single inbound HTTP(S) request received by tpp::http_server.
 * @note plaintext HTTP without SSL is also supported via a "downgrade"
 * setting
 */
class TPP_EXPORT http_server_request : public ssl_connection {
  /**
   * @brief The request body, e.g. form data
   */
  std::string request_body;

  /**
   * @brief Headers from the client
   */
  http_headers request_headers;

  /**
   * @brief Time at which the request should be abandoned
   */
  time_t timeout;

  /**
   * @brief Headers for our response
   */
  http_headers response_headers;

  /**
   * @brief Handler to handle the inbound request
   */
  http_server_request_event handler {};

  /**
   * @brief Response body
   */
  std::string response_body;

 protected:
  /**
   * @brief The type of the request, e.g. GET, POST
   */
  std::string request_type;

  /**
   * @brief Path part of the request, including any query string
   */
  std::string path;

  /**
   * @brief Current connection state
   */
  http_state state;

  /**
   * @brief HTTP status code for the response
   */
  uint16_t status {0};

  /**
   * @brief Start the connection
   */
  virtual void connect() override;

  /**
   * @brief Called when the output buffer has been fully flushed to the
   * socket. Used to defer closing the connection until the response has
   * actually been sent.
   */
  virtual void on_buffer_drained() override;

  /**
   * @brief Maximum size of POST body
   */
  [[nodiscard]] uint64_t get_max_post_size() const;

  /**
   * @brief Maximum size of headers
   */
  [[nodiscard]] uint64_t get_max_header_size() const;

  /**
   * @brief Reply with an error message
   * @param error_code error code
   * @param message message
   */
  void generate_error(uint16_t error_code, const std::string &message);

 public:
  /**
   * @brief If true the response timed out while waiting
   */
  bool timed_out;

  /**
   * @brief Get request path, including any query string
   * @return request path
   */
  [[nodiscard]] std::string get_path() const;

  /**
   * @brief Get request state
   * @return request state
   */
  [[nodiscard]] http_state get_state() const;

  /**
   * @brief Get current response body
   * @return response body
   */
  [[nodiscard]] std::string get_response_body() const;

  /**
   * @brief Get current request body
   * @return request body
   */
  [[nodiscard]] std::string get_request_body() const;

  /**
   * @brief Get current status code
   * @return status code
   */
  [[nodiscard]] uint16_t get_status() const;

  /**
   * @brief Content length sent by client
   */
  uint64_t content_length {ULLONG_MAX};

  /**
   * @brief Constructs a server request object for an incoming connection.
   * @param creator creating owner
   * @param fd file descriptor
   * @param port Port the connection came in on
   * @param plaintext_downgrade true if plaintext, false if SSL
   * @param private_key if SSL, the path to the private key PEM
   * @param public_key if SSL, the path to the public key PEM
   * @param handle_request request handler callback
   */
  http_server_request(application *creator, socket fd, uint16_t port,
                      bool plaintext_downgrade, const std::string &private_key,
                      const std::string        &public_key,
                      http_server_request_event handle_request);

  virtual ~http_server_request() override;

  /**
   * @brief Processes incoming data from the SSL socket input buffer.
   * @param buffer The buffer contents. Can modify this value removing the
   * head elements when processed.
   */
  virtual bool handle_buffer(std::string &buffer) override;

  /**
   * @brief Close HTTP(S) socket
   */
  virtual void close() override;

  /**
   * @brief Fires every second from the underlying socket I/O loop, used for
   * timeouts
   */
  virtual void one_second_timer() override;

  /**
   * @brief Get a HTTP request header
   * @param header_name Header name to find, case insensitive
   * @return Header content or empty string if not found. If multiple
   * values have the same header_name, this will return one of them.
   */
  [[nodiscard]] const std::string get_header(
      const std::string &header_name) const;

  /**
   * @brief Get the number of headers with the same header name
   * @param header_name
   * @return the number of headers with this name
   */
  [[nodiscard]] size_t get_header_count(const std::string &header_name) const;

  /**
   * @brief Get a set of HTTP request headers with a common name
   * @param header_name
   * @return A list of headers with the same name, or an empty list if not
   * found
   */
  [[nodiscard]] std::list<std::string> get_header_list(
      const std::string &header_name) const;

  /**
   * @brief Get all HTTP request headers
   * @return headers as a map
   */
  [[nodiscard]] http_headers get_headers() const;

  /**
   * @brief Set a response header
   * @param header header name
   * @param value header value
   * @return ref to self
   */
  http_server_request &set_response_header(const std::string &header,
                                           const std::string &value);

  /**
   * @brief Set the response body
   * @param new_content response content
   * @return ref to self
   */
  http_server_request &set_response_body(const std::string &new_content);

  /**
   * @brief Set the response HTTP status, e.g. 200 for OK, 404 for not
   * found, 429 for rate limited etc.
   * @param new_status HTTP status
   * @return ref to self
   */
  http_server_request &set_status(uint16_t new_status);

  /**
   * @brief Get the whole response, headers and body, as a string ready to
   * write to the socket.
   */
  [[nodiscard]] std::string get_response();
};

}// namespace tpp
