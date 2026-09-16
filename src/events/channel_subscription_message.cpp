#include <tpp/event.h>

namespace tpp::events {

void channel_subscription_message::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
