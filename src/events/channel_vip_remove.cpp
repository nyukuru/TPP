#include <tpp/event.h>

namespace tpp::events {

void channel_vip_remove::handle(session *, nlohmann::json &,
                                const std::string &) const {
}

}// namespace tpp::events
