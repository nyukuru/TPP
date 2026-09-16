#include <tpp/event.h>

namespace tpp::events {

void automod_message_update::handle(session *, nlohmann::json &,
                                    const std::string &) const {
}

}// namespace tpp::events
