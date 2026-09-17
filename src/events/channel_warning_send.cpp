#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_warning_send::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_warning_send.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_warning_send_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.target.fill_from_json(j, "user");
    event.reason = string_not_null(&j, "reason");
    for_each_json(&j, "chat_rules_cited", [&event](nlohmann::json *elem) {
      if (elem->is_string()) {
        event.chat_rules_cited.push_back(elem->get<std::string>());
      }
    });

    creator->on_channel_warning_send.call(event);
  });
}

}// namespace tpp::events
