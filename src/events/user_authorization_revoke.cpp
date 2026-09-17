#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void user_authorization_revoke::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_user_authorization_revoke.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    user_authorization_revoke_t event(creator, shard_id, raw);
    event.client_id = string_not_null(&j, "client_id");
    event.revoked.fill_from_json(j, "user");

    creator->on_user_authorization_revoke.call(event);
  });
}

}// namespace tpp::events
