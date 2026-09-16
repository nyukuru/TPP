#include <tpp/event.h>

namespace tpp::events {

void channel_moderator_add::handle(session *, nlohmann::json &,
                                   const std::string &) const {
}

}// namespace tpp::events
