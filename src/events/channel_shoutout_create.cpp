#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_shoutout_create::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_shoutout_create.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_shoutout_create_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.to_broadcaster.fill_from_json(j, "to_broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.viewer_count = int64_not_null(&j, "viewer_count");
    event.started_at = string_not_null(&j, "started_at");
    event.cooldown_ends_at = string_not_null(&j, "cooldown_ends_at");
    event.target_cooldown_ends_at = string_not_null(&j, "target_cooldown_ends_at");

    creator->on_channel_shoutout_create.call(event);
  });
}

}// namespace tpp::events
