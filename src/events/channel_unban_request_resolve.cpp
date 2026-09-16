#include <tpp/event.h>

namespace tpp::events {

void channel_unban_request_resolve::handle(session *, nlohmann::json &,
                                           const std::string &) const {
}

}// namespace tpp::events
