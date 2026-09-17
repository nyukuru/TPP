#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void automod_terms_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_automod_terms_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    automod_terms_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.action = string_not_null(&j, "action");
    event.from_automod = bool_not_null(&j, "from_automod");
    for_each_json(&j, "terms", [&event](nlohmann::json *term) {
      if (term->is_string()) {
        event.terms.push_back(term->get<std::string>());
      }
    });

    creator->on_automod_terms_update.call(event);
  });
}

}// namespace tpp::events
