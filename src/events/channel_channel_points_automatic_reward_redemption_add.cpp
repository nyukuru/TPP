#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_channel_points_automatic_reward_redemption_add::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_channel_points_automatic_reward_redemption_add.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_channel_points_automatic_reward_redemption_add_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.redeemer.fill_from_json(j, "user");
    event.reward_id = string_not_null(&j, "reward_id");
    event.reward_title = string_not_null(&j, "reward_title");
    event.reward_prompt = string_not_null(&j, "reward_prompt");
    event.reward_cost = int64_not_null(&j, "reward_cost");
    event.user_input = string_not_null(&j, "user_input");
    event.redeemed_at = string_not_null(&j, "redeemed_at");

    creator->on_channel_channel_points_automatic_reward_redemption_add.call(event);
  });
}

}// namespace tpp::events
