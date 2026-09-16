#include <tpp/event.h>

namespace tpp::events {

void user_update::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
