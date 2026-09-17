#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void user_whisper_message::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_user_whisper_message.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    user_whisper_message_t event(creator, shard_id, raw);
    event.from_user.fill_from_json(j, "from_user");
    event.to_user.fill_from_json(j, "to_user");
    event.whisper_id = string_not_null(&j, "whisper_id");
    nlohmann::json &whisper = j["whisper"];
    event.text = string_not_null(&whisper, "text");

    creator->on_user_whisper_message.call(event);
  });
}

}// namespace tpp::events
