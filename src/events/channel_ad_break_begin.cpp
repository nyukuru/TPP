#include <tpp/event.h>

namespace tpp::events {

void channel_ad_break_begin::handle(session *, nlohmann::json &,
                                    const std::string &) const {
}

}// namespace tpp::events
