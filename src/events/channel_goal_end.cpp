#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_goal_end::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_goal_end.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_goal_end_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.type = string_not_null(&j, "type");
    event.description = string_not_null(&j, "description");
    event.current_amount = int64_not_null(&j, "current_amount");
    event.target_amount = int64_not_null(&j, "target_amount");
    event.started_at = string_not_null(&j, "started_at");
    event.ended_at = string_not_null(&j, "ended_at");
    event.is_achieved = bool_not_null(&j, "is_achieved");

    creator->on_channel_goal_end.call(event);
  });
}

}// namespace tpp::events
