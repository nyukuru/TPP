#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_ban::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_ban.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_ban_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.target.fill_from_json(j, "user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.reason = string_not_null(&j, "reason");
    event.banned_at = string_not_null(&j, "banned_at");
    event.ends_at = string_not_null(&j, "ends_at");
    event.is_permanent = bool_not_null(&j, "is_permanent");

    creator->on_channel_ban.call(event);
  });
}

}// namespace tpp::events
