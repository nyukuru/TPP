#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_charity_campaign_donate::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_charity_campaign_donate.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_charity_campaign_donate_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.campaign_id = string_not_null(&j, "campaign_id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.donor.fill_from_json(j, "user");
    event.charity_name = string_not_null(&j, "charity_name");
    event.charity_description = string_not_null(&j, "charity_description");
    event.charity_logo = string_not_null(&j, "charity_logo");
    event.charity_website = string_not_null(&j, "charity_website");
    nlohmann::json &amount = j["amount"];
    event.amount_value = int64_not_null(&amount, "value");
    event.amount_decimal_places = int64_not_null(&amount, "decimal_places");
    event.amount_currency = string_not_null(&amount, "currency");

    creator->on_channel_charity_campaign_donate.call(event);
  });
}

}// namespace tpp::events
