#include <tpp/event.h>

namespace tpp::events {

void channel_goal_progress::handle(session *, nlohmann::json &,
                                   const std::string &) const {
}

}// namespace tpp::events
