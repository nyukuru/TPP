#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void drop_entitlement_grant::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_drop_entitlement_grant.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    /* Twitch nests the entitlement fields under "data" for this
     * subscription type; fall back to the top level if a caller has
     * already unwrapped it. */
    nlohmann::json data = j["data"];
    if (data.empty()) {
      data = j;
    }

    drop_entitlement_grant_t event(creator, shard_id, raw);
    event.organization_id = string_not_null(&data, "organization_id");
    event.category_id = string_not_null(&data, "category_id");
    event.category_name = string_not_null(&data, "category_name");
    event.campaign_id = string_not_null(&data, "campaign_id");
    event.recipient.fill_from_json(data, "user");
    event.entitlement_id = string_not_null(&data, "entitlement_id");
    event.benefit_id = string_not_null(&data, "benefit_id");
    event.created_at = string_not_null(&data, "created_at");

    creator->on_drop_entitlement_grant.call(event);
  });
}

}// namespace tpp::events
