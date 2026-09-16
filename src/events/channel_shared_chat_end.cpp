#include <tpp/event.h>

namespace tpp::events {

void channel_shared_chat_end::handle(session *, nlohmann::json &,
                                     const std::string &) const {
}

}// namespace tpp::events
