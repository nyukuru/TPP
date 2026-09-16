#include <tpp/event.h>

namespace tpp::events {

void channel_poll_end::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
