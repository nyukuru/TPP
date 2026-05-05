/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
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

#ifdef _WIN32
#  include <WS2tcpip.h>
#  include <WinSock2.h>
#  include <io.h>
// clang-format off
   namespace tpp::compat {
     using pollfd = WSAPOLLFD;
     inline int poll(pollfd *fds, ULONG nfds, int timeout) {
       return WSAPoll(fds, nfds, timeout);
     }
   }
// clang-format on
#  pragma comment(lib, "ws2_32")
#else
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <poll.h>
#  include <sys/socket.h>
// clang-format off
   namespace tpp::compat {
     using ::poll;
     using ::pollfd;
   }
// clang-format on
#endif
