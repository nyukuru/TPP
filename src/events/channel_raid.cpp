#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_raid::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_raid.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_raid_t event(creator, shard_id, raw);
    event.from_broadcaster.fill_from_json(j, "from_broadcaster_user");
    event.to_broadcaster.fill_from_json(j, "to_broadcaster_user");
    event.viewers = int64_not_null(&j, "viewers");

    creator->on_channel_raid.call(event);
  });
}

}// namespace tpp::events
