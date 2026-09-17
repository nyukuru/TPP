#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_prediction_end::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_prediction_end.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_prediction_end_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.title = string_not_null(&j, "title");
    event.winning_outcome_id = string_not_null(&j, "winning_outcome_id");
    for_each_json(&j, "outcomes", [&event](nlohmann::json *elem) {
      prediction_outcome o;
      o.fill_from_json(elem);
      event.outcomes.push_back(o);
    });
    event.created_at = string_not_null(&j, "created_at");
    event.ended_at = string_not_null(&j, "ended_at");
    event.ended_reason = string_not_null(&j, "ended_reason");

    creator->on_channel_prediction_end.call(event);
  });
}

}// namespace tpp::events
