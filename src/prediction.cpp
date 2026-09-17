#include <tpp/event.h>
#include <tpp/prediction.h>

namespace tpp {

prediction_top_predictor &prediction_top_predictor::fill_from_json_impl(nlohmann::json *j) {
  predictor.fill_from_json(*j, "user");
  channel_points_won = int64_not_null(j, "channel_points_won");
  channel_points_used = int64_not_null(j, "channel_points_used");
  return *this;
}

prediction_outcome &prediction_outcome::fill_from_json_impl(nlohmann::json *j) {
  id = string_not_null(j, "id");
  title = string_not_null(j, "title");
  color = string_not_null(j, "color");
  users = int64_not_null(j, "users");
  channel_points = int64_not_null(j, "channel_points");
  for_each_json(j, "top_predictors", [this](nlohmann::json *elem) {
    prediction_top_predictor p;
    p.fill_from_json(elem);
    top_predictors.push_back(p);
  });
  return *this;
}

}// namespace tpp
