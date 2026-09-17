#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_guest_star_settings_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_guest_star_settings_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_guest_star_settings_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.is_moderator_send_live_enabled = bool_not_null(&j, "is_moderator_send_live_enabled");
    event.slot_count = int64_not_null(&j, "slot_count");
    event.is_browser_source_audio_enabled = bool_not_null(&j, "is_browser_source_audio_enabled");
    event.group_layout = string_not_null(&j, "group_layout");

    creator->on_channel_guest_star_settings_update.call(event);
  });
}

}// namespace tpp::events
