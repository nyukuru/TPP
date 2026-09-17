#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.title = string_not_null(&j, "title");
    event.language = string_not_null(&j, "language");
    event.category_id = string_not_null(&j, "category_id");
    event.category_name = string_not_null(&j, "category_name");
    for_each_json(&j, "content_classification_labels", [&event](nlohmann::json *label) {
      if (label->is_string()) {
        event.content_classification_labels.push_back(label->get<std::string>());
      }
    });

    creator->on_channel_update.call(event);
  });
}

}// namespace tpp::events
