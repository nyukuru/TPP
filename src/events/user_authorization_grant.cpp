#include <tpp/event.h>

namespace tpp::events {

void user_authorization_grant::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
