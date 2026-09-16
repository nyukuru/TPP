#include <tpp/event.h>

namespace tpp::events {

void channel_subscription_end::handle(session *, nlohmann::json &,
                                      const std::string &) const {
}

}// namespace tpp::events
