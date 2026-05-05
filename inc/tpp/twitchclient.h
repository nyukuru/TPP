#pragma once

#include <string>

namespace tpp {
class TwitchClient {
  std::string client_id;

  std::string client_secret;

  std::string bot_id;

  std::string redirect_uri;
};

}// namespace tpp
