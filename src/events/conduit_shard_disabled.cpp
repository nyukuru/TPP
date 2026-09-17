#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void conduit_shard_disabled::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_conduit_shard_disabled.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    conduit_shard_disabled_t event(creator, shard_id, raw);
    event.conduit_id = string_not_null(&j, "conduit_id");
    event.shard_id = string_not_null(&j, "shard_id");
    event.status = string_not_null(&j, "status");
    event.transport.fill_from_json(&j["transport"]);

    creator->on_conduit_shard_disabled.call(event);
  });
}

}// namespace tpp::events
