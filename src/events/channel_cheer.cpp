#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_cheer::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_cheer.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_cheer_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.chatter.fill_from_json(j, "user");
    event.is_anonymous = bool_not_null(&j, "is_anonymous");
    event.message = string_not_null(&j, "message");
    event.bits = int64_not_null(&j, "bits");

    creator->on_channel_cheer.call(event);
  });
}

}// namespace tpp::events
