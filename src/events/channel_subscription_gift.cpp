#include <tpp/event.h>

namespace tpp::events {

void channel_subscription_gift::handle(session *, nlohmann::json &,
                                       const std::string &) const {
}

}// namespace tpp::events
