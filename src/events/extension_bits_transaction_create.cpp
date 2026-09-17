#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void extension_bits_transaction_create::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_extension_bits_transaction_create.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    extension_bits_transaction_create_t event(creator, shard_id, raw);
    event.id = string_not_null(&j, "id");
    event.extension_client_id = string_not_null(&j, "extension_client_id");
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.purchaser.fill_from_json(j, "user");
    nlohmann::json &product = j["product"];
    event.product_name = string_not_null(&product, "name");
    event.product_sku = string_not_null(&product, "sku");
    event.product_bits = int64_not_null(&product, "bits");
    event.product_in_development = bool_not_null(&product, "in_development");

    creator->on_extension_bits_transaction_create.call(event);
  });
}

}// namespace tpp::events
