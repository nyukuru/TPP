#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_shared_chat_end::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_shared_chat_end.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_shared_chat_end_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.session_id = string_not_null(&j, "session_id");
    event.host_broadcaster.fill_from_json(j, "host_broadcaster_user");

    creator->on_channel_shared_chat_end.call(event);
  });
}

}// namespace tpp::events
