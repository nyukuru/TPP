#include <tpp/event.h>

namespace tpp::events {

void channel_chat_message_delete::handle(session *, nlohmann::json &,
                                         const std::string &) const {
}

}// namespace tpp::events
