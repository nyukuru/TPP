#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_moderator_remove::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_moderator_remove.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_moderator_remove_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.moderator.fill_from_json(j, "user");

    creator->on_channel_moderator_remove.call(event);
  });
}

}// namespace tpp::events
