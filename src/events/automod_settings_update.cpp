#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void automod_settings_update::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_automod_settings_update.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    automod_settings_update_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.moderator.fill_from_json(j, "moderator_user");
    event.overall_level = static_cast<int>(int64_not_null(&j, "overall_level"));
    event.bullying = static_cast<int>(int64_not_null(&j, "bullying"));
    event.disability = static_cast<int>(int64_not_null(&j, "disability"));
    event.race_ethnicity_or_religion = static_cast<int>(int64_not_null(&j, "race_ethnicity_or_religion"));
    event.misogyny = static_cast<int>(int64_not_null(&j, "misogyny"));
    event.sexuality_sex_or_gender = static_cast<int>(int64_not_null(&j, "sexuality_sex_or_gender"));
    event.aggression = static_cast<int>(int64_not_null(&j, "aggression"));
    event.sex_based_terms = static_cast<int>(int64_not_null(&j, "sex_based_terms"));
    event.swearing = static_cast<int>(int64_not_null(&j, "swearing"));

    creator->on_automod_settings_update.call(event);
  });
}

}// namespace tpp::events
