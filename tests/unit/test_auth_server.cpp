#include <gtest/gtest.h>
#include <tpp/auth_server.h>
#include <tpp/conduit.h>
#include <tpp/consumer.h>
#include <tpp/scope.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
/* Ports are picked per-test to avoid TIME_WAIT collisions between tests
 * that bind and quickly tear down a local listener on the same port. */
uint16_t next_test_port() {
  static std::atomic<uint16_t> port {24000};
  return port.fetch_add(1);
}
}// namespace

class AuthServerTest : public ::testing::Test {
 protected:
  const std::string client_id = "test_client_id";
  tpp::conduit app {client_id};
};

TEST_F(AuthServerTest, ConstructorWithValidPortSucceeds) {
  EXPECT_NO_THROW({ tpp::auth_server auth(&app, next_test_port()); });
}

TEST_F(AuthServerTest, ConstructorZeroPortThrows) {
  EXPECT_THROW({ tpp::auth_server auth(&app, 0); }, std::invalid_argument);
}

TEST_F(AuthServerTest, InitiallyNotRunning) {
  tpp::auth_server auth(&app, next_test_port());
  EXPECT_FALSE(auth.is_running());
}

TEST_F(AuthServerTest, StartBeforeConduitRunningThrows) {
  tpp::auth_server auth(&app, next_test_port());
  EXPECT_THROW({ auth.start(); }, std::runtime_error);
}

TEST_F(AuthServerTest, StartAfterConduitRunningSucceeds) {
  app.start(false);
  tpp::auth_server auth(&app, next_test_port());

  auth.start();
  EXPECT_TRUE(auth.is_running());

  auth.stop();
  EXPECT_FALSE(auth.is_running());

  app.stop();
}

TEST_F(AuthServerTest, OnAuthenticateEmptyByDefault) {
  tpp::auth_server auth(&app, next_test_port());
  EXPECT_TRUE(auth.on_authenticate.empty());
}

TEST_F(AuthServerTest, OnAuthenticateNotEmptyAfterAttaching) {
  tpp::auth_server auth(&app, next_test_port());
  auth.on_authenticate([](const std::shared_ptr<tpp::consumer> &) {});
  EXPECT_FALSE(auth.on_authenticate.empty());
}

TEST_F(AuthServerTest, OnAuthenticateSupportsMultipleHandlers) {
  tpp::auth_server auth(&app, next_test_port());
  auto consumer = app.create_consumer("1", "u", "tok");

  int calls_to_first = 0;
  int calls_to_second = 0;
  auth.on_authenticate([&](const std::shared_ptr<tpp::consumer> &) { ++calls_to_first; });
  auth.on_authenticate([&](const std::shared_ptr<tpp::consumer> &) { ++calls_to_second; });

  auth.on_authenticate.call(consumer);

  EXPECT_EQ(calls_to_first, 1);
  EXPECT_EQ(calls_to_second, 1);
}

TEST_F(AuthServerTest, GenerateAuthUrlContainsExpectedComponents) {
  tpp::auth_server auth(&app, next_test_port());
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_chat_edit);

  std::string url = auth.generate_auth_url(scopes);

  EXPECT_NE(url.find("https://id.twitch.tv/oauth2/authorize"), std::string::npos);
  EXPECT_NE(url.find("response_type=token"), std::string::npos);
  EXPECT_EQ(url.find("id_token"), std::string::npos) << "response_type should not request an id_token without the openid "
                                                        "scope";
  EXPECT_NE(url.find("client_id=" + client_id), std::string::npos);
  EXPECT_NE(url.find("chat%3Aread"), std::string::npos);
  EXPECT_NE(url.find("chat%3Aedit"), std::string::npos);
  EXPECT_NE(url.find("&state="), std::string::npos);
  EXPECT_EQ(url.find("&nonce="), std::string::npos);
  EXPECT_EQ(url.find("&claims="), std::string::npos);
}

TEST_F(AuthServerTest, GenerateAuthUrlWithOpenidRequestsIdToken) {
  tpp::auth_server auth(&app, next_test_port());
  tpp::scope scopes(tpp::scope::s_chat_read, tpp::scope::s_openid);

  std::string url = auth.generate_auth_url(scopes);

  EXPECT_NE(url.find("response_type=token+id_token"), std::string::npos);
  EXPECT_NE(url.find("&nonce="), std::string::npos);
}

TEST_F(AuthServerTest, GenerateAuthUrlIncludesClaimsWhenGiven) {
  tpp::auth_server auth(&app, next_test_port());
  tpp::scope scopes(tpp::scope::s_chat_read);

  std::string url = auth.generate_auth_url(scopes, R"({"userinfo":{"email":null}})");

  EXPECT_NE(url.find("&claims="), std::string::npos);
}

TEST_F(AuthServerTest, GenerateAuthUrlStateChangesEachCall) {
  tpp::auth_server auth(&app, next_test_port());
  tpp::scope scopes(tpp::scope::s_chat_read);

  std::string url1 = auth.generate_auth_url(scopes);
  std::string url2 = auth.generate_auth_url(scopes);

  auto extract_state = [](const std::string &url) {
    auto pos = url.find("&state=");
    return url.substr(pos);
  };
  EXPECT_NE(extract_state(url1), extract_state(url2));
}
