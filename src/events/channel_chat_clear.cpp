#include <tpp/event.h>

namespace tpp::events {

void channel_chat_clear::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
