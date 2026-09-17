#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void automod_message_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_automod_message_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    automod_message_update_t event(creator, shard_id, raw);
    event.msg.fill_from_json(j, "user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.status = string_not_null(&j, "status");
    event.category = string_not_null(&j, "category");
    event.level = static_cast<int>(int64_not_null(&j, "level"));
    event.held_at = string_not_null(&j, "held_at");
    event.reason = string_not_null(&j, "reason");
    event.automod.fill_from_json(&j["automod"]);
    event.blocked_term.fill_from_json(&j["blocked_term"]);

    creator->on_automod_message_update.call(event);
  });
}

}// namespace tpp::events
