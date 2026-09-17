#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_moderate::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_moderate.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_moderate_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.source_broadcaster.fill_from_json(j, "source_broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.action = string_not_null(&j, "action");
    event.action_metadata.fill_from_json(&j[event.action]);

    creator->on_channel_moderate.call(event);
  });
}

}// namespace tpp::events
