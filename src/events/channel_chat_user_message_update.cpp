#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_chat_user_message_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_chat_user_message_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_chat_user_message_update_t event(creator, shard_id, raw);
    event.msg.fill_from_json(j, "user");
    event.status = string_not_null(&j, "status");

    creator->on_channel_chat_user_message_update.call(event);
  });
}

}// namespace tpp::events
