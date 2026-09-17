#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_prediction_begin::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_prediction_begin.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_prediction_begin_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.title = string_not_null(&j, "title");
    for_each_json(&j, "outcomes", [&event](nlohmann::json *elem) {
      prediction_outcome o;
      o.fill_from_json(elem);
      event.outcomes.push_back(o);
    });
    event.prediction_window_seconds = int64_not_null(&j, "prediction_window_seconds");
    event.created_at = string_not_null(&j, "created_at");
    event.locks_at = string_not_null(&j, "locks_at");

    creator->on_channel_prediction_begin.call(event);
  });
}

}// namespace tpp::events
