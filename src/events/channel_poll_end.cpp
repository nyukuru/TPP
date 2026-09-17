#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_poll_end::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_poll_end.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_poll_end_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.title = string_not_null(&j, "title");
    for_each_json(&j, "choices", [&event](nlohmann::json *elem) {
      poll_choice c;
      c.fill_from_json(elem);
      event.choices.push_back(c);
    });
    event.bits_voting.fill_from_json(&j["bits_voting"]);
    event.channel_points_voting.fill_from_json(&j["channel_points_voting"]);
    event.started_at = string_not_null(&j, "started_at");
    event.ended_at = string_not_null(&j, "ended_at");
    event.status = string_not_null(&j, "status");

    creator->on_channel_poll_end.call(event);
  });
}

}// namespace tpp::events
