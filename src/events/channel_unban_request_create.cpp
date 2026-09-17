#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_unban_request_create::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_unban_request_create.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_unban_request_create_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.target.fill_from_json(j, "user");
    event.text = string_not_null(&j, "text");
    event.created_at = string_not_null(&j, "created_at");

    creator->on_channel_unban_request_create.call(event);
  });
}

}// namespace tpp::events
