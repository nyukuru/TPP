#include <tpp/event.h>

namespace tpp::events {

void channel_goal_end::handle(session *, nlohmann::json &,
                              const std::string &) const {
}

}// namespace tpp::events
