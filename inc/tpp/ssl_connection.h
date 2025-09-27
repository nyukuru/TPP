#include <openssl/ssl.h>
#include <openssl/sslerr.h>

#pragma once

namespace tpp {

struct ssl_context {
  ssl_context(client)
};

class ssl_connection {
  ssl_connection(client *creator, const std::string &_hostname,
                 const std::string &_port = "443");

}:

}// namespace tpp

