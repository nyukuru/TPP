#include <cstdlib>
#include <iostream>

#include "tpp/cluster.h"

int main() {
  // Get the token from environment variable (similar to DPP examples)
  const char* token_env = std::getenv("TWITCH_TOKEN");
  if (!token_env) {
    std::cerr << "Please set TWITCH_TOKEN environment variable" << std::endl;
    return 1;
  }

  try {
    // Create cluster with chat messages and follows intents
    tpp::cluster bot(token_env,
                     tpp::intents::chat_messages | tpp::intents::follows);

    std::cout << "Starting Twitch bot..." << std::endl;
    std::cout << "Token: " << bot.get_token() << std::endl;
    std::cout << "Bot is running: " << (bot.is_running() ? "Yes" : "No")
              << std::endl;

    // Join a channel
    bot.join_channel("testchannel");

    // Send a message
    bot.send_message("testchannel", "Hello from TPP bot!");

    // Start the bot (non-blocking for this example)
    bot.start(false);

    std::cout << "Bot is running: " << (bot.is_running() ? "Yes" : "No")
              << std::endl;

    // In a real application, you might want to keep the bot running
    // and handle events, but for this example we'll stop it quickly
    std::this_thread::sleep_for(std::chrono::seconds(1));

    bot.stop();
    std::cout << "Bot stopped." << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}