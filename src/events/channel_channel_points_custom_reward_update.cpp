#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_channel_points_custom_reward_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_channel_points_custom_reward_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_channel_points_custom_reward_update_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.is_enabled = bool_not_null(&j, "is_enabled");
    event.is_paused = bool_not_null(&j, "is_paused");
    event.is_in_stock = bool_not_null(&j, "is_in_stock");
    event.title = string_not_null(&j, "title");
    event.cost = int64_not_null(&j, "cost");
    event.prompt = string_not_null(&j, "prompt");
    event.is_user_input_required = bool_not_null(&j, "is_user_input_required");
    event.should_redemptions_skip_request_queue = bool_not_null(&j, "should_redemptions_skip_request_queue");
    event.cooldown_expires_at = string_not_null(&j, "cooldown_expires_at");
    event.redemptions_redeemed_current_stream = int64_not_null(&j, "redemptions_redeemed_current_stream");
    event.max_per_stream.fill_from_json(&j["max_per_stream"]);
    event.max_per_user_per_stream.fill_from_json(&j["max_per_user_per_stream"]);
    event.global_cooldown.fill_from_json(&j["global_cooldown"]);
    event.background_color = string_not_null(&j, "background_color");
    event.image.fill_from_json(&j["image"]);
    event.default_image.fill_from_json(&j["default_image"]);

    creator->on_channel_channel_points_custom_reward_update.call(event);
  });
}

}// namespace tpp::events
