#include <tpp/event.h>

namespace tpp::events {

void channel_follow::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
