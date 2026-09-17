#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_shoutout_receive::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_shoutout_receive.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_shoutout_receive_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.from_broadcaster.fill_from_json(j, "from_broadcaster_user");
    event.viewer_count = int64_not_null(&j, "viewer_count");
    event.started_at = string_not_null(&j, "started_at");

    creator->on_channel_shoutout_receive.call(event);
  });
}

}// namespace tpp::events
