#include <tpp/event.h>

namespace tpp::events {

void channel_moderator_remove::handle(session *, nlohmann::json &,
                                      const std::string &) const {
}

}// namespace tpp::events
