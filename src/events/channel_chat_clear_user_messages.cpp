#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_chat_clear_user_messages::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_chat_clear_user_messages.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_chat_clear_user_messages_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.target.fill_from_json(j, "target_user");

    creator->on_channel_chat_clear_user_messages.call(event);
  });
}

}// namespace tpp::events
