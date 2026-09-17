#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void user_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_user_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    user_update_t event(creator, shard_id, raw);
    event.updated.fill_from_json(j, "user");
    event.email = string_not_null(&j, "email");
    event.email_verified = bool_not_null(&j, "email_verified");
    event.description = string_not_null(&j, "description");

    creator->on_user_update.call(event);
  });
}

}// namespace tpp::events
