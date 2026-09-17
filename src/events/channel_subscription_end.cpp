#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_subscription_end::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_subscription_end.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_subscription_end_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.subscriber.fill_from_json(j, "user");
    event.tier = string_not_null(&j, "tier");
    event.is_gift = bool_not_null(&j, "is_gift");

    creator->on_channel_subscription_end.call(event);
  });
}

}// namespace tpp::events
