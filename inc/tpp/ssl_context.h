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

#include <cstdint>
#include <string>

namespace tpp::detail {

struct wrapped_ssl_ctx;

/**
 * @brief Gets or creates a wrapped SSL context. Port 0 is the single
 * client context shared by all SSL client connections; other ports each
 * get their own server context with its own loaded certificate.
 * @param port Port number. Pass zero to create or get the client context.
 * @param private_key Private key PEM pathname for server contexts
 * @param public_key Public key PEM pathname for server contexts
 * @return wrapped SSL context
 */
TPP_EXPORT wrapped_ssl_ctx *generate_ssl_context(
    uint16_t port = 0, const std::string &private_key = "",
    const std::string &public_key = "");

/**
 * @brief Releases an SSL context.
 * @warning Only call this once no SSL connections using it remain.
 * @param port port number to release
 */
TPP_EXPORT void release_ssl_context(uint16_t port = 0);

};// namespace tpp::detail
