#include <tpp/event.h>

namespace tpp::events {

void channel_ban::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
