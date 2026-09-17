#include <tpp/event.h>
#include <tpp/message.h>

namespace tpp {

chat_fragment &chat_fragment::fill_from_json(nlohmann::json &j) {
  type = string_not_null(&j, "type");
  text = string_not_null(&j, "text");

  if (type == "cheermote") {
    nlohmann::json &cheermote = j["cheermote"];
    cheermote_prefix = string_not_null(&cheermote, "prefix");
    cheermote_bits = int64_not_null(&cheermote, "bits");
    cheermote_tier = int64_not_null(&cheermote, "tier");
  } else if (type == "emote") {
    emote_data.fill_from_json(&j["emote"]);
  } else if (type == "mention") {
    nlohmann::json &mention = j["mention"];
    mentioned_user.fill_from_json(mention, "user");
  }

  return *this;
}

message &message::fill_from_json(nlohmann::json &j, const std::string &chatter_prefix) {
  broadcaster.fill_from_json(j, "broadcaster_user");
  chatter.fill_from_json(j, chatter_prefix);
  id = string_not_null(&j, "message_id");

  nlohmann::json &msg = j["message"];
  text = string_not_null(&msg, "text");
  for_each_json(&msg, "fragments", [this](nlohmann::json *elem) {
    chat_fragment frag;
    frag.fill_from_json(*elem);
    fragments.push_back(frag);
  });

  return *this;
}

}// namespace tpp
