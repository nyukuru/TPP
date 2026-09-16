#include <tpp/event.h>

namespace tpp::events {

void channel_warning_send::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
