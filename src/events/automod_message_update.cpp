#include <tpp/event.h>

namespace tpp::events {

void automod_message_update::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
