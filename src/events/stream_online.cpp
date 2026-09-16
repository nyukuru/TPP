#include <tpp/event.h>

namespace tpp::events {

void stream_online::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
