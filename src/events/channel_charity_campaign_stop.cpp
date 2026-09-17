#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_charity_campaign_stop::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_charity_campaign_stop.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_charity_campaign_stop_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.charity_name = string_not_null(&j, "charity_name");
    event.charity_description = string_not_null(&j, "charity_description");
    event.charity_logo = string_not_null(&j, "charity_logo");
    event.charity_website = string_not_null(&j, "charity_website");
    nlohmann::json &current_amount = j["current_amount"];
    event.current_amount_value = int64_not_null(&current_amount, "value");
    event.current_amount_decimal_places = int64_not_null(&current_amount, "decimal_places");
    event.current_amount_currency = string_not_null(&current_amount, "currency");
    nlohmann::json &target_amount = j["target_amount"];
    event.target_amount_value = int64_not_null(&target_amount, "value");
    event.target_amount_decimal_places = int64_not_null(&target_amount, "decimal_places");
    event.target_amount_currency = string_not_null(&target_amount, "currency");
    event.stopped_at = string_not_null(&j, "stopped_at");

    creator->on_channel_charity_campaign_stop.call(event);
  });
}

}// namespace tpp::events
