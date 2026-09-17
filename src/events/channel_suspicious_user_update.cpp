#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_suspicious_user_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_suspicious_user_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_suspicious_user_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.target.fill_from_json(j, "user");
    event.low_trust_status = string_not_null(&j, "low_trust_status");

    creator->on_channel_suspicious_user_update.call(event);
  });
}

}// namespace tpp::events
