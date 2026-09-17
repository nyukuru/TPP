#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_chat_message::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_chat_message.empty())
    return;

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    chat_message_t event(creator, shard_id, raw);
    event.msg.fill_from_json(j);

    creator->on_chat_message.call(event);
  });
}

}// namespace tpp::events
