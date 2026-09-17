#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_ad_break_begin::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_ad_break_begin.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_ad_break_begin_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.requester.fill_from_json(j, "requester_user");
    event.duration_seconds = int64_not_null(&j, "duration_seconds");
    event.started_at = string_not_null(&j, "started_at");
    event.is_automatic = bool_not_null(&j, "is_automatic");

    creator->on_channel_ad_break_begin.call(event);
  });
}

}// namespace tpp::events
