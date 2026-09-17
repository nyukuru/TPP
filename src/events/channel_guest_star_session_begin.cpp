#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_guest_star_session_begin::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_guest_star_session_begin.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_guest_star_session_begin_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.session_id = string_not_null(&j, "session_id");
    event.started_at = string_not_null(&j, "started_at");

    creator->on_channel_guest_star_session_begin.call(event);
  });
}

}// namespace tpp::events
