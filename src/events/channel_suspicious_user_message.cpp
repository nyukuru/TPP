#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_suspicious_user_message::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_suspicious_user_message.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_suspicious_user_message_t event(creator, shard_id, raw);
    event.broadcaster.fill_from_json(j, "broadcaster_user");
    event.target.fill_from_json(j, "user");
    event.low_trust_status = string_not_null(&j, "low_trust_status");
    for_each_json(&j, "shared_ban_channel_ids", [&event](nlohmann::json *elem) {
      if (elem->is_string()) {
        event.shared_ban_channel_ids.push_back(elem->get<std::string>());
      }
    });
    for_each_json(&j, "types", [&event](nlohmann::json *elem) {
      if (elem->is_string()) {
        event.types.push_back(elem->get<std::string>());
      }
    });
    event.ban_evasion_evaluation = string_not_null(&j, "ban_evasion_evaluation");
    nlohmann::json &message = j["message"];
    event.message_id = string_not_null(&message, "message_id");
    event.message_text = string_not_null(&message, "text");
    for_each_json(&message, "fragments", [&event](nlohmann::json *elem) {
      chat_fragment frag;
      frag.fill_from_json(*elem);
      event.message_fragments.push_back(frag);
    });

    creator->on_channel_suspicious_user_message.call(event);
  });
}

}// namespace tpp::events
