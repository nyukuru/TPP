#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_subscription_gift::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_subscription_gift.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_subscription_gift_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.gifter.fill_from_json(j, "user");
    event.is_anonymous = bool_not_null(&j, "is_anonymous");
    event.total = int64_not_null(&j, "total");
    event.tier = string_not_null(&j, "tier");
    auto cumulative_it = j.find("cumulative_total");
    event.cumulative_total_is_null = cumulative_it == j.end() || cumulative_it->is_null();
    event.cumulative_total = int64_not_null(&j, "cumulative_total");

    creator->on_channel_subscription_gift.call(event);
  });
}

}// namespace tpp::events
