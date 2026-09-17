#include <tpp/event.h>
#include <tpp/poll.h>

namespace tpp {

poll_choice &poll_choice::fill_from_json_impl(nlohmann::json *j) {
  id = string_not_null(j, "id");
  title = string_not_null(j, "title");
  bits_votes = int64_not_null(j, "bits_votes");
  channel_points_votes = int64_not_null(j, "channel_points_votes");
  votes = int64_not_null(j, "votes");
  return *this;
}

poll_voting &poll_voting::fill_from_json_impl(nlohmann::json *j) {
  is_enabled = bool_not_null(j, "is_enabled");
  amount_per_vote = int64_not_null(j, "amount_per_vote");
  return *this;
}

}// namespace tpp
