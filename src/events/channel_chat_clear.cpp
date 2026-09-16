#include <tpp/event.h>

namespace tpp::events {

void channel_chat_clear::handle(session *, nlohmann::json &,
                                const std::string &) const {
}

}// namespace tpp::events
