#include <gtest/gtest.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/event.h>
#include <tpp/eventsub_client.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

namespace {

/**
 * @brief handle() defers all parsing onto the conduit's dispatch thread
 * pool (see events::event_handler::handle()), so tests need a running
 * conduit and a way to wait for the async router call rather than
 * asserting immediately after handle_event() returns.
 */
class EventTest : public ::testing::Test {
 protected:
  tpp::conduit app {"test_client_id"};

  void SetUp() override {
    /* app.socketengine only exists once running, and eventsub_client's
     * constructor needs it - construct client here, after start(), not
     * as a member initializer (which would run before SetUp()). */
    app.start(false);
    /* Points nowhere reachable - only used as a routing target/pointer
     * and for its creator field, never expected to actually connect - it
     * legitimately gets an async connection error, which the IO thread
     * processes concurrently with anything this test does on the main
     * thread. Constructing/destroying it via app.defer() (like
     * conduit::wire_shard_callbacks() does for shard replacement) runs
     * that on the IO thread instead, so it can never race the IO
     * thread's own handling of that same object. */
    run_on_io_thread([&] { client = std::make_unique<tpp::eventsub_client>(&app, "wss://127.0.0.1:1/"); });
  }

  void TearDown() override {
    /* Any eventsub_client constructed directly against app must be
     * destroyed before app.stop() - see conduit::socketengine's note. */
    run_on_io_thread([&] { client.reset(); });
    app.stop();
  }

  /* Runs fn on the conduit's IO thread via defer(), blocking until it
   * completes. */
  template<typename Fn>
  void run_on_io_thread(Fn fn) {
    std::mutex m;
    std::condition_variable cv;
    bool done = false;
    app.defer([&] {
      fn();
      std::lock_guard<std::mutex> lock(m);
      done = true;
      cv.notify_one();
    });
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [&] { return done; });
  }

  std::unique_ptr<tpp::eventsub_client> client;

  template<typename Predicate>
  bool wait_for(std::mutex &m, std::condition_variable &cv, Predicate pred) {
    std::unique_lock<std::mutex> lock(m);
    return cv.wait_for(lock, std::chrono::seconds(2), pred);
  }
};

}// namespace

TEST_F(EventTest, HandleEventDispatchesToOnChatMessageHook) {
  auto consumer = std::make_shared<tpp::consumer>(&app, "1971641", "streamer", "tok_abc");
  app.add_consumer(consumer);

  std::mutex m;
  std::condition_variable cv;
  bool called = false;
  tpp::chat_message_t received;

  app.on_chat_message([&](const tpp::chat_message_t &event) {
    std::lock_guard<std::mutex> lock(m);
    received = event;
    called = true;
    cv.notify_one();
  });

  std::string event_json = R"({
    "broadcaster_user_id": "1971641",
    "broadcaster_user_login": "streamer",
    "broadcaster_user_name": "streamer",
    "chatter_user_id": "4145994",
    "chatter_user_login": "viewer32",
    "chatter_user_name": "viewer32",
    "message": {"text": "ping"}
  })";
  nlohmann::json event = nlohmann::json::parse(event_json);
  tpp::events::handle_event(client.get(), "channel.chat.message", event, event_json);

  ASSERT_TRUE(wait_for(m, cv, [&] { return called; }));
  EXPECT_EQ(received.owner, &app);
  EXPECT_EQ(app.get_consumer(received.msg.broadcaster.id), consumer);
  EXPECT_EQ(received.msg.broadcaster.id, "1971641");
  EXPECT_EQ(received.msg.chatter.id, "4145994");
  EXPECT_EQ(received.msg.text, "ping");
}

TEST_F(EventTest, HandleEventOnUnknownSubscriptionTypeDoesNotCrash) {
  nlohmann::json event = nlohmann::json::parse("{}");
  EXPECT_NO_THROW({ tpp::events::handle_event(client.get(), "not.a.real.type", event, "{}"); });
}

TEST_F(EventTest, HandleEventFiresEvenWithNoConsumerTracked) {
  /* No add_consumer() call this time - handlers no longer resolve a
   * consumer automatically at all, so this should fire regardless. */
  std::mutex m;
  std::condition_variable cv;
  bool called = false;
  tpp::chat_message_t received;

  app.on_chat_message([&](const tpp::chat_message_t &event) {
    std::lock_guard<std::mutex> lock(m);
    received = event;
    called = true;
    cv.notify_one();
  });

  std::string event_json = R"({"broadcaster_user_id": "1971641", "message": {"text": "ping"}})";
  nlohmann::json event = nlohmann::json::parse(event_json);
  tpp::events::handle_event(client.get(), "channel.chat.message", event, event_json);

  ASSERT_TRUE(wait_for(m, cv, [&] { return called; }));
  EXPECT_EQ(received.owner, &app);
}

TEST_F(EventTest, ChatMessageHandlerWithMissingFieldsYieldsEmptyFields) {
  auto consumer = std::make_shared<tpp::consumer>(&app, "1", "u", "tok");
  app.add_consumer(consumer);

  std::mutex m;
  std::condition_variable cv;
  bool called = false;
  tpp::chat_message_t received;

  app.on_chat_message([&](const tpp::chat_message_t &event) {
    std::lock_guard<std::mutex> lock(m);
    received = event;
    called = true;
    cv.notify_one();
  });

  const tpp::events::event_handler *handler = tpp::events::find_handler("channel.chat.message");
  ASSERT_NE(handler, nullptr);

  nlohmann::json event = nlohmann::json::parse(R"({"broadcaster_user_id": "1", "message":{}})");
  EXPECT_NO_THROW({ handler->handle(client.get(), event, "{}"); });

  ASSERT_TRUE(wait_for(m, cv, [&] { return called; }));
  EXPECT_EQ(received.msg.broadcaster.id, "1");
  EXPECT_EQ(received.msg.text, "");
}
