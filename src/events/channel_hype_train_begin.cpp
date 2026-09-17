#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_hype_train_begin::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_hype_train_begin.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_hype_train_begin_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.total = int64_not_null(&j, "total");
    event.progress = int64_not_null(&j, "progress");
    event.goal = int64_not_null(&j, "goal");
    for_each_json(&j, "top_contributions", [&event](nlohmann::json *elem) {
      hype_train_contribution c;
      c.fill_from_json(elem);
      event.top_contributions.push_back(c);
    });
    event.last_contribution.fill_from_json(&j["last_contribution"]);
    event.level = int64_not_null(&j, "level");
    event.is_shared_train = bool_not_null(&j, "is_shared_train");
    event.started_at = string_not_null(&j, "started_at");
    event.expires_at = string_not_null(&j, "expires_at");

    creator->on_channel_hype_train_begin.call(event);
  });
}

}// namespace tpp::events
