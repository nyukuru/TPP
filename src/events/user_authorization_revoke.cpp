#include <tpp/event.h>

namespace tpp::events {

void user_authorization_revoke::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
