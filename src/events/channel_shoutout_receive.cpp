#include <tpp/event.h>

namespace tpp::events {

void channel_shoutout_receive::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
