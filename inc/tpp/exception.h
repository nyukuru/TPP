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

#include <exception>
#include <string>

namespace tpp {

/**
 * @brief Exception error codes possible for tpp::exception::code().
 */
enum exception_error_code {
  err_no_code_specified   = 0,
  err_ssl_new             = 1,
  err_ssl_connect         = 2,
  err_write               = 3,
  err_ssl_write           = 4,
  err_bind_failure        = 8,
  err_nonblocking_failure = 9,
  err_connect_failure     = 11,
  err_ssl_context         = 12,
  err_ssl_version         = 13,
  err_invalid_socket      = 14,
  err_socket_error        = 15,
};

/**
 * @brief The tpp::exception class derives from std::exception and supports
 * some other ways of passing in error details such as via std::string.
 */
class TPP_EXPORT exception : public std::exception {
 protected:
  /**
   * @brief Exception message
   */
  std::string msg;

  /**
   * @brief Exception error code
   */
  exception_error_code error_code {err_no_code_specified};

 public:
  exception() = default;

  explicit exception(const char *what) : msg(what) {
  }

  exception(exception_error_code code, const char *what)
      : msg(what), error_code(code) {
  }

  explicit exception(const std::string &what) : msg(what) {
  }

  exception(exception_error_code code, const std::string &what)
      : msg(what), error_code(code) {
  }

  exception(const exception &)                = default;
  exception(exception &&) noexcept            = default;
  ~exception() override                       = default;
  exception &operator=(const exception &)     = default;
  exception &operator=(exception &&) noexcept = default;

  /**
   * @brief Get exception message
   * @return const char* error message
   */
  [[nodiscard]] const char *what() const noexcept override {
    return msg.c_str();
  }

  /**
   * @brief Get exception code
   * @return exception_error_code error code
   */
  [[nodiscard]] exception_error_code code() const noexcept {
    return error_code;
  }
};

/**
 * @brief Thrown when a TCP/SSL connection could not be established or
 * maintained.
 */
class TPP_EXPORT connection_exception : public tpp::exception {
 public:
  using tpp::exception::exception;
};

}// namespace tpp
