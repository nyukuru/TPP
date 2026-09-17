#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

#include <map>
#include <string>
#include <utility>

namespace tpp {

event_dispatch_t::event_dispatch_t(conduit *creator, uint32_t shard_id, const std::string &raw) : raw_event(raw), shard(shard_id), owner(creator) {
}

event_dispatch_t::event_dispatch_t(conduit *creator, uint32_t shard_id, std::string &&raw) : raw_event(std::move(raw)), shard(shard_id), owner(creator) {
}

eventsub_client *event_dispatch_t::from() const {
  return owner->get_shard(shard);
}

void for_each_json(nlohmann::json *parent, std::string_view key, const std::function<void(nlohmann::json *)> &fn) {
  auto it = parent->find(key);
  if (it == parent->end() || it->is_null()) {
    return;
  }
  for (nlohmann::json &elem : *it) {
    fn(&elem);
  }
}

std::string string_not_null(const nlohmann::json *j, const char *keyname) {
  /* Returns empty string if the value is not a string, or is null or not defined */
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && k->is_string() ? k->get<std::string>() : "";
  } else {
    return const_cast<char *>("");
  }
}

void set_string_not_null(const nlohmann::json *j, const char *keyname, std::string &v) {
  /* Returns empty string if the value is not a string, or is null or not defined */
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && k->is_string() ? k->get<std::string>() : "";
  }
}

double double_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && !k->is_string() ? k->get<double>() : 0;
  } else {
    return 0;
  }
}

void set_double_not_null(const nlohmann::json *j, const char *keyname, double &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && !k->is_string() ? k->get<double>() : 0;
  }
}

uint64_t int64_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && !k->is_string() ? k->get<uint64_t>() : 0;
  } else {
    return 0;
  }
}

void set_int64_not_null(const nlohmann::json *j, const char *keyname, uint64_t &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && !k->is_string() ? k->get<uint64_t>() : 0;
  }
}

uint32_t int32_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && !k->is_string() ? k->get<uint32_t>() : 0;
  } else {
    return 0;
  }
}

void set_int32_not_null(const nlohmann::json *j, const char *keyname, uint32_t &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && !k->is_string() ? k->get<uint32_t>() : 0;
  }
}

uint16_t int16_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && !k->is_string() ? k->get<uint16_t>() : 0;
  } else {
    return 0;
  }
}

void set_int16_not_null(const nlohmann::json *j, const char *keyname, uint16_t &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && !k->is_string() ? k->get<uint16_t>() : 0;
  }
}

uint8_t int8_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() && !k->is_string() ? k->get<uint8_t>() : 0;
  } else {
    return 0;
  }
}

void set_int8_not_null(const nlohmann::json *j, const char *keyname, uint8_t &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() && !k->is_string() ? k->get<uint8_t>() : 0;
  }
}

bool bool_not_null(const nlohmann::json *j, const char *keyname) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    return !k->is_null() ? (k->get<bool>() == true) : false;
  } else {
    return false;
  }
}

void set_bool_not_null(const nlohmann::json *j, const char *keyname, bool &v) {
  auto k = j->find(keyname);
  if (k != j->end()) {
    v = !k->is_null() ? (k->get<bool>() == true) : false;
  }
}

event event::channel_chat_message(const std::string &user_id) {
  return event {"channel.chat.message", "1", {{"broadcaster_user_id", user_id}, {"user_id", user_id}}};
}

event event::automod_message_hold(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"automod.message.hold", "2", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::automod_message_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"automod.message.update", "2", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::automod_settings_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"automod.settings.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::automod_terms_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"automod.terms.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_update(const std::string &broadcaster_user_id) {
  return event {"channel.update", "2", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_follow(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.follow", "2", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_ad_break_begin(const std::string &broadcaster_user_id) {
  return event {"channel.ad_break.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_chat_clear(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.clear", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_clear_user_messages(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.clear_user_messages", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_message_delete(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.message_delete", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_notification(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.notification", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_settings_update(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat_settings.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_user_message_hold(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.user_message_hold", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_chat_user_message_update(const std::string &broadcaster_user_id, const std::string &user_id) {
  return event {"channel.chat.user_message_update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"user_id", user_id}}};
}

event event::channel_shared_chat_begin(const std::string &broadcaster_user_id) {
  return event {"channel.shared_chat.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_shared_chat_update(const std::string &broadcaster_user_id) {
  return event {"channel.shared_chat.update", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_shared_chat_end(const std::string &broadcaster_user_id) {
  return event {"channel.shared_chat.end", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_subscribe(const std::string &broadcaster_user_id) {
  return event {"channel.subscribe", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_subscription_end(const std::string &broadcaster_user_id) {
  return event {"channel.subscription.end", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_subscription_gift(const std::string &broadcaster_user_id) {
  return event {"channel.subscription.gift", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_subscription_message(const std::string &broadcaster_user_id) {
  return event {"channel.subscription.message", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_cheer(const std::string &broadcaster_user_id) {
  return event {"channel.cheer", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_raid(const std::string &to_broadcaster_user_id) {
  return event {"channel.raid", "1", {{"to_broadcaster_user_id", to_broadcaster_user_id}}};
}

event event::channel_ban(const std::string &broadcaster_user_id) {
  return event {"channel.ban", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_unban(const std::string &broadcaster_user_id) {
  return event {"channel.unban", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_unban_request_create(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.unban_request.create", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_unban_request_resolve(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.unban_request.resolve", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_moderate(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.moderate", "2", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_moderator_add(const std::string &broadcaster_user_id) {
  return event {"channel.moderator.add", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_moderator_remove(const std::string &broadcaster_user_id) {
  return event {"channel.moderator.remove", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_guest_star_session_begin(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.guest_star_session.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_guest_star_session_end(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.guest_star_session.end", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_guest_star_guest_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.guest_star_guest.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_guest_star_settings_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.guest_star_settings.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_channel_points_automatic_reward_redemption_add(const std::string &broadcaster_user_id) {
  return event {"channel.channel_points_automatic_reward_redemption.add", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_channel_points_custom_reward_add(const std::string &broadcaster_user_id) {
  return event {"channel.channel_points_custom_reward.add", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_channel_points_custom_reward_update(const std::string &broadcaster_user_id, const std::string &reward_id) {
  std::map<std::string, std::string> condition {{"broadcaster_user_id", broadcaster_user_id}};
  if (!reward_id.empty()) {
    condition["reward_id"] = reward_id;
  }
  return event {"channel.channel_points_custom_reward.update", "1", condition};
}

event event::channel_channel_points_custom_reward_remove(const std::string &broadcaster_user_id, const std::string &reward_id) {
  std::map<std::string, std::string> condition {{"broadcaster_user_id", broadcaster_user_id}};
  if (!reward_id.empty()) {
    condition["reward_id"] = reward_id;
  }
  return event {"channel.channel_points_custom_reward.remove", "1", condition};
}

event event::channel_channel_points_custom_reward_redemption_add(const std::string &broadcaster_user_id, const std::string &reward_id) {
  std::map<std::string, std::string> condition {{"broadcaster_user_id", broadcaster_user_id}};
  if (!reward_id.empty()) {
    condition["reward_id"] = reward_id;
  }
  return event {"channel.channel_points_custom_reward_redemption.add", "1", condition};
}

event event::channel_channel_points_custom_reward_redemption_update(const std::string &broadcaster_user_id, const std::string &reward_id) {
  std::map<std::string, std::string> condition {{"broadcaster_user_id", broadcaster_user_id}};
  if (!reward_id.empty()) {
    condition["reward_id"] = reward_id;
  }
  return event {"channel.channel_points_custom_reward_redemption.update", "1", condition};
}

event event::channel_poll_begin(const std::string &broadcaster_user_id) {
  return event {"channel.poll.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_poll_progress(const std::string &broadcaster_user_id) {
  return event {"channel.poll.progress", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_poll_end(const std::string &broadcaster_user_id) {
  return event {"channel.poll.end", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_prediction_begin(const std::string &broadcaster_user_id) {
  return event {"channel.prediction.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_prediction_progress(const std::string &broadcaster_user_id) {
  return event {"channel.prediction.progress", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_prediction_lock(const std::string &broadcaster_user_id) {
  return event {"channel.prediction.lock", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_prediction_end(const std::string &broadcaster_user_id) {
  return event {"channel.prediction.end", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_suspicious_user_message(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.suspicious_user.message", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_suspicious_user_update(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.suspicious_user.update", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_vip_add(const std::string &broadcaster_user_id) {
  return event {"channel.vip.add", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_vip_remove(const std::string &broadcaster_user_id) {
  return event {"channel.vip.remove", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_warning_acknowledge(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.warning.acknowledge", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_warning_send(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.warning.send", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_charity_campaign_donate(const std::string &broadcaster_user_id) {
  return event {"channel.charity_campaign.donate", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_charity_campaign_start(const std::string &broadcaster_user_id) {
  return event {"channel.charity_campaign.start", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_charity_campaign_progress(const std::string &broadcaster_user_id) {
  return event {"channel.charity_campaign.progress", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_charity_campaign_stop(const std::string &broadcaster_user_id) {
  return event {"channel.charity_campaign.stop", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_goal_begin(const std::string &broadcaster_user_id) {
  return event {"channel.goal.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_goal_progress(const std::string &broadcaster_user_id) {
  return event {"channel.goal.progress", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_goal_end(const std::string &broadcaster_user_id) {
  return event {"channel.goal.end", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_hype_train_begin(const std::string &broadcaster_user_id) {
  return event {"channel.hype_train.begin", "2", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_hype_train_progress(const std::string &broadcaster_user_id) {
  return event {"channel.hype_train.progress", "2", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_hype_train_end(const std::string &broadcaster_user_id) {
  return event {"channel.hype_train.end", "2", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::channel_shield_mode_begin(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.shield_mode.begin", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_shield_mode_end(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.shield_mode.end", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_shoutout_create(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.shoutout.create", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::channel_shoutout_receive(const std::string &broadcaster_user_id, const std::string &moderator_user_id) {
  return event {"channel.shoutout.receive", "1", {{"broadcaster_user_id", broadcaster_user_id}, {"moderator_user_id", moderator_user_id}}};
}

event event::conduit_shard_disabled(const std::string &client_id, const std::string &conduit_id) {
  std::map<std::string, std::string> condition {{"client_id", client_id}};
  if (!conduit_id.empty()) {
    condition["conduit_id"] = conduit_id;
  }
  return event {"conduit.shard.disabled", "1", condition};
}

event event::drop_entitlement_grant(const std::string &organization_id, const std::string &category_id, const std::string &campaign_id) {
  std::map<std::string, std::string> condition {{"organization_id", organization_id}};
  if (!category_id.empty()) {
    condition["category_id"] = category_id;
  }
  if (!campaign_id.empty()) {
    condition["campaign_id"] = campaign_id;
  }
  return event {"drop.entitlement.grant", "1", condition};
}

event event::extension_bits_transaction_create(const std::string &extension_client_id) {
  return event {"extension.bits_transaction.create", "1", {{"extension_client_id", extension_client_id}}};
}

event event::stream_online(const std::string &broadcaster_user_id) {
  return event {"stream.online", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::stream_offline(const std::string &broadcaster_user_id) {
  return event {"stream.offline", "1", {{"broadcaster_user_id", broadcaster_user_id}}};
}

event event::user_authorization_grant(const std::string &client_id) {
  return event {"user.authorization.grant", "1", {{"client_id", client_id}}};
}

event event::user_authorization_revoke(const std::string &client_id) {
  return event {"user.authorization.revoke", "1", {{"client_id", client_id}}};
}

event event::user_update(const std::string &user_id) {
  return event {"user.update", "1", {{"user_id", user_id}}};
}

event event::user_whisper_message(const std::string &user_id) {
  return event {"user.whisper.message", "1", {{"user_id", user_id}}};
}

/**
 * @brief Constructs one static, stateless instance of an event handler
 * type and returns a pointer to it - mirrors DPP's own make_static_event
 * helper. Handlers carry no state, so one shared instance per type,
 * living for the process's lifetime, is all event_map needs.
 */
template<typename T>
events::event_handler *make_static_event() {
  static T static_event;
  return &static_event;
}

/**
 * @brief Maps every known EventSub subscription type to its handler.
 * Mirrors DPP's own event_map (see dpp::discord_client::handle_event).
 */
const std::map<std::string, events::event_handler *> event_map = {
    {"automod.message.hold", make_static_event<events::automod_message_hold>()},
    {"automod.message.update", make_static_event<events::automod_message_update>()},
    {"automod.settings.update", make_static_event<events::automod_settings_update>()},
    {"automod.terms.update", make_static_event<events::automod_terms_update>()},
    {"channel.update", make_static_event<events::channel_update>()},
    {"channel.follow", make_static_event<events::channel_follow>()},
    {"channel.ad_break.begin", make_static_event<events::channel_ad_break_begin>()},
    {"channel.chat.clear", make_static_event<events::channel_chat_clear>()},
    {"channel.chat.clear_user_messages", make_static_event<events::channel_chat_clear_user_messages>()},
    {"channel.chat.message", make_static_event<events::channel_chat_message>()},
    {"channel.chat.message_delete", make_static_event<events::channel_chat_message_delete>()},
    {"channel.chat.notification", make_static_event<events::channel_chat_notification>()},
    {"channel.chat_settings.update", make_static_event<events::channel_chat_settings_update>()},
    {"channel.chat.user_message_hold", make_static_event<events::channel_chat_user_message_hold>()},
    {"channel.chat.user_message_update", make_static_event<events::channel_chat_user_message_update>()},
    {"channel.shared_chat.begin", make_static_event<events::channel_shared_chat_begin>()},
    {"channel.shared_chat.update", make_static_event<events::channel_shared_chat_update>()},
    {"channel.shared_chat.end", make_static_event<events::channel_shared_chat_end>()},
    {"channel.subscribe", make_static_event<events::channel_subscribe>()},
    {"channel.subscription.end", make_static_event<events::channel_subscription_end>()},
    {"channel.subscription.gift", make_static_event<events::channel_subscription_gift>()},
    {"channel.subscription.message", make_static_event<events::channel_subscription_message>()},
    {"channel.cheer", make_static_event<events::channel_cheer>()},
    {"channel.raid", make_static_event<events::channel_raid>()},
    {"channel.ban", make_static_event<events::channel_ban>()},
    {"channel.unban", make_static_event<events::channel_unban>()},
    {"channel.unban_request.create", make_static_event<events::channel_unban_request_create>()},
    {"channel.unban_request.resolve", make_static_event<events::channel_unban_request_resolve>()},
    {"channel.moderate", make_static_event<events::channel_moderate>()},
    {"channel.moderator.add", make_static_event<events::channel_moderator_add>()},
    {"channel.moderator.remove", make_static_event<events::channel_moderator_remove>()},
    {"channel.guest_star_session.begin", make_static_event<events::channel_guest_star_session_begin>()},
    {"channel.guest_star_session.end", make_static_event<events::channel_guest_star_session_end>()},
    {"channel.guest_star_guest.update", make_static_event<events::channel_guest_star_guest_update>()},
    {"channel.guest_star_settings.update", make_static_event<events::channel_guest_star_settings_update>()},
    {"channel.channel_points_automatic_reward_redemption.add", make_static_event<events::channel_channel_points_automatic_reward_redemption_add>()},
    {"channel.channel_points_custom_reward.add", make_static_event<events::channel_channel_points_custom_reward_add>()},
    {"channel.channel_points_custom_reward.update", make_static_event<events::channel_channel_points_custom_reward_update>()},
    {"channel.channel_points_custom_reward.remove", make_static_event<events::channel_channel_points_custom_reward_remove>()},
    {"channel.channel_points_custom_reward_redemption.add", make_static_event<events::channel_channel_points_custom_reward_redemption_add>()},
    {"channel.channel_points_custom_reward_redemption.update", make_static_event<events::channel_channel_points_custom_reward_redemption_update>()},
    {"channel.poll.begin", make_static_event<events::channel_poll_begin>()},
    {"channel.poll.progress", make_static_event<events::channel_poll_progress>()},
    {"channel.poll.end", make_static_event<events::channel_poll_end>()},
    {"channel.prediction.begin", make_static_event<events::channel_prediction_begin>()},
    {"channel.prediction.progress", make_static_event<events::channel_prediction_progress>()},
    {"channel.prediction.lock", make_static_event<events::channel_prediction_lock>()},
    {"channel.prediction.end", make_static_event<events::channel_prediction_end>()},
    {"channel.suspicious_user.message", make_static_event<events::channel_suspicious_user_message>()},
    {"channel.suspicious_user.update", make_static_event<events::channel_suspicious_user_update>()},
    {"channel.vip.add", make_static_event<events::channel_vip_add>()},
    {"channel.vip.remove", make_static_event<events::channel_vip_remove>()},
    {"channel.warning.acknowledge", make_static_event<events::channel_warning_acknowledge>()},
    {"channel.warning.send", make_static_event<events::channel_warning_send>()},
    {"channel.charity_campaign.donate", make_static_event<events::channel_charity_campaign_donate>()},
    {"channel.charity_campaign.start", make_static_event<events::channel_charity_campaign_start>()},
    {"channel.charity_campaign.progress", make_static_event<events::channel_charity_campaign_progress>()},
    {"channel.charity_campaign.stop", make_static_event<events::channel_charity_campaign_stop>()},
    {"channel.goal.begin", make_static_event<events::channel_goal_begin>()},
    {"channel.goal.progress", make_static_event<events::channel_goal_progress>()},
    {"channel.goal.end", make_static_event<events::channel_goal_end>()},
    {"channel.hype_train.begin", make_static_event<events::channel_hype_train_begin>()},
    {"channel.hype_train.progress", make_static_event<events::channel_hype_train_progress>()},
    {"channel.hype_train.end", make_static_event<events::channel_hype_train_end>()},
    {"channel.shield_mode.begin", make_static_event<events::channel_shield_mode_begin>()},
    {"channel.shield_mode.end", make_static_event<events::channel_shield_mode_end>()},
    {"channel.shoutout.create", make_static_event<events::channel_shoutout_create>()},
    {"channel.shoutout.receive", make_static_event<events::channel_shoutout_receive>()},
    {"conduit.shard.disabled", make_static_event<events::conduit_shard_disabled>()},
    {"drop.entitlement.grant", make_static_event<events::drop_entitlement_grant>()},
    {"extension.bits_transaction.create", make_static_event<events::extension_bits_transaction_create>()},
    {"stream.online", make_static_event<events::stream_online>()},
    {"stream.offline", make_static_event<events::stream_offline>()},
    {"user.authorization.grant", make_static_event<events::user_authorization_grant>()},
    {"user.authorization.revoke", make_static_event<events::user_authorization_revoke>()},
    {"user.update", make_static_event<events::user_update>()},
    {"user.whisper.message", make_static_event<events::user_whisper_message>()},
};

void events::handle_event(eventsub_client *client, const std::string &subscription_type, nlohmann::json &j, const std::string &raw) {
  auto it = event_map.find(subscription_type);
  if (it != event_map.end()) {
    /* A handler with nullptr would be silently ignored, same as DPP -
     * event_map has none today since every subscription type TPP knows
     * about already has a (possibly no-op) handler class, but a future
     * type added to the map before its handler exists could use one. */
    if (it->second != nullptr) {
      it->second->handle(client, j, raw);
    }
  } else {
    client->creator->log(ll_debug, "Unhandled event: " + subscription_type + ", " + j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
  }
}

const events::event_handler *events::find_handler(const std::string &subscription_type) {
  auto it = event_map.find(subscription_type);
  return it == event_map.end() ? nullptr : it->second;
}

}// namespace tpp
