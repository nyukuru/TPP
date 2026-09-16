#include <tpp/event.h>

namespace tpp::events {

void extension_bits_transaction_create::handle(session *, nlohmann::json &,
                                               const std::string &) const {
}

}// namespace tpp::events
