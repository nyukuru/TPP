#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void stream_online::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_stream_online.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    stream_online_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.type = string_not_null(&j, "type");
    event.started_at = string_not_null(&j, "started_at");

    creator->on_stream_online.call(event);
  });
}

}// namespace tpp::events
