#include <tpp/event.h>

namespace tpp::events {

void conduit_shard_disabled::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
