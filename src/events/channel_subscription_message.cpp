#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_subscription_message::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_subscription_message.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_subscription_message_t event(creator, shard_id, raw);
    event.msg.fill_from_json(j, "user");
    event.tier = string_not_null(&j, "tier");
    event.cumulative_months = int64_not_null(&j, "cumulative_months");
    auto streak_it = j.find("streak_months");
    event.streak_months_is_null = streak_it == j.end() || streak_it->is_null();
    event.streak_months = int64_not_null(&j, "streak_months");
    event.duration_months = int64_not_null(&j, "duration_months");

    creator->on_channel_subscription_message.call(event);
  });
}

}// namespace tpp::events
