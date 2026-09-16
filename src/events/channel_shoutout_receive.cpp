#include <tpp/event.h>

namespace tpp::events {

void channel_shoutout_receive::handle(session *, nlohmann::json &,
                                      const std::string &) const {
}

}// namespace tpp::events
