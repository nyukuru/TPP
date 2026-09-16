#include <tpp/event.h>

namespace tpp::events {

void conduit_shard_disabled::handle(session *, nlohmann::json &,
                                    const std::string &) const {
}

}// namespace tpp::events
