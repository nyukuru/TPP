#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_chat_settings_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_chat_settings_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_chat_settings_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.emote_mode = bool_not_null(&j, "emote_mode");
    event.follower_mode = bool_not_null(&j, "follower_mode");
    event.follower_mode_duration_minutes = int64_not_null(&j, "follower_mode_duration_minutes");
    event.slow_mode = bool_not_null(&j, "slow_mode");
    event.slow_mode_wait_time_seconds = int64_not_null(&j, "slow_mode_wait_time_seconds");
    event.subscriber_mode = bool_not_null(&j, "subscriber_mode");
    event.unique_chat_mode = bool_not_null(&j, "unique_chat_mode");

    creator->on_channel_chat_settings_update.call(event);
  });
}

}// namespace tpp::events
