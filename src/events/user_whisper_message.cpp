#include <tpp/event.h>

namespace tpp::events {

void user_whisper_message::handle(consumer *, nlohmann::json &, const std::string &) const {
}

}// namespace tpp::events
