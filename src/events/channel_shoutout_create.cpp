#include <tpp/event.h>

namespace tpp::events {

void channel_shoutout_create::handle(session *, nlohmann::json &,
                                     const std::string &) const {
}

}// namespace tpp::events
