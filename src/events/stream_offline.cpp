#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void stream_offline::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_stream_offline.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    stream_offline_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");

    creator->on_stream_offline.call(event);
  });
}

}// namespace tpp::events
