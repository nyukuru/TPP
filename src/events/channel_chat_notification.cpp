#include <tpp/conduit.h>
#include <tpp/dispatcher.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

namespace tpp::events {

void channel_chat_notification::handle(eventsub_client *client, nlohmann::json &j, const std::string &raw) const {
  conduit *creator = client->creator;
  if (creator->on_channel_chat_notification.empty()) {
    return;
  }

  uint32_t shard_id = client->shard_id;
  creator->enqueue_dispatch([creator, shard_id, j, raw]() mutable {
    channel_chat_notification_t event(creator, shard_id, raw);
    event.msg.fill_from_json(j);
    event.chatter_is_anonymous = bool_not_null(&j, "chatter_is_anonymous");
    event.color = string_not_null(&j, "color");
    for_each_json(&j, "badges", [&event](nlohmann::json *elem) {
      chat_badge b;
      b.fill_from_json(elem);
      event.badges.push_back(b);
    });
    event.system_message = string_not_null(&j, "system_message");
    event.notice_type = string_not_null(&j, "notice_type");
    event.notice_metadata.fill_from_json(&j[event.notice_type]);

    creator->on_channel_chat_notification.call(event);
  });
}

}// namespace tpp::events
