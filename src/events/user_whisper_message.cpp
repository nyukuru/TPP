#include <tpp/event.h>

namespace tpp::events {

void user_whisper_message::handle(session *, nlohmann::json &,
                                  const std::string &) const {
}

}// namespace tpp::events
