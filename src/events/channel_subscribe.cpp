#include <tpp/event.h>

namespace tpp::events {

void channel_subscribe::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
