#include <tpp/event.h>

namespace tpp::events {

void drop_entitlement_grant::handle(session *, nlohmann::json &,
                                    const std::string &) const {
}

}// namespace tpp::events
