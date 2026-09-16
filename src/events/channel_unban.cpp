#include <tpp/event.h>

namespace tpp::events {

void channel_unban::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
