#include <tpp/event.h>

namespace tpp::events {

void channel_chat_settings_update::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
