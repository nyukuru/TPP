#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_unban_request_resolve::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_unban_request_resolve.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_unban_request_resolve_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    /* Unlike most other event types, the moderator here is keyed
     * "moderator_id"/"moderator_login"/"moderator_name", not
     * "moderator_user_id" and friends. */
    event.moderator.fill_from_json(j, "moderator");
    event.target.fill_from_json(j, "user");
    event.resolution_text = string_not_null(&j, "resolution_text");
    event.status = string_not_null(&j, "status");

    creator->on_channel_unban_request_resolve.call(event);
  });
}

}// namespace tpp::events
