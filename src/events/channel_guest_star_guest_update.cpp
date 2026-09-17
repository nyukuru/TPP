#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_guest_star_guest_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_guest_star_guest_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_guest_star_guest_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.session_id = string_not_null(&j, "session_id");
    event.moderator.fill_from_json(j, "moderator_user");
    event.guest.fill_from_json(j, "guest_user");
    event.slot_id = string_not_null(&j, "slot_id");
    event.state = string_not_null(&j, "state");
    event.host.fill_from_json(j, "host_user");
    event.host_video_enabled = bool_not_null(&j, "host_video_enabled");
    event.host_audio_enabled = bool_not_null(&j, "host_audio_enabled");
    event.host_volume = int64_not_null(&j, "host_volume");

    creator->on_channel_guest_star_guest_update.call(event);
  });
}

}// namespace tpp::events
