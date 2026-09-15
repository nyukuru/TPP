#include <gtest/gtest.h>
#include <internal/utility.h>

#include <string>

using tpp::utility::lowercase;
using tpp::utility::parse_query_string;
using tpp::utility::to_hex;
using tpp::utility::tokenize;
using tpp::utility::uppercase;
using tpp::utility::url_decode;
using tpp::utility::url_encode;

TEST(Utility, TokenizeSplitsOnWhitespace) {
  auto tokens = tokenize("GET /foo HTTP/1.1", " ");
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "GET");
  EXPECT_EQ(tokens[1], "/foo");
  EXPECT_EQ(tokens[2], "HTTP/1.1");
}

TEST(Utility, TokenizeCollapsesRepeatedSeparators) {
  auto tokens = tokenize("a  b   c", " ");
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "a");
  EXPECT_EQ(tokens[1], "b");
  EXPECT_EQ(tokens[2], "c");
}

TEST(Utility, TokenizeEmptyString) {
  EXPECT_TRUE(tokenize("", " ").empty());
}

TEST(Utility, TokenizeDefaultSeparatorSplitsLines) {
  auto tokens = tokenize("line1\r\nline2\r\nline3");
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "line1");
  EXPECT_EQ(tokens[2], "line3");
}

TEST(Utility, LowercaseUppercaseRoundtrip) {
  EXPECT_EQ(lowercase("Content-Type"), "content-type");
  EXPECT_EQ(uppercase("get"), "GET");
  EXPECT_EQ(lowercase("ALREADY_LOWER_NOT"), "already_lower_not");
  EXPECT_EQ(lowercase(""), "");
}

TEST(Utility, UrlEncodeLeavesUnreservedCharactersAlone) {
  EXPECT_EQ(url_encode("abcXYZ019-_.~"), "abcXYZ019-_.~");
}

TEST(Utility, UrlEncodeEscapesReservedCharacters) {
  EXPECT_EQ(url_encode("chat:read chat:edit"), "chat%3Aread%20chat%3Aedit");
  EXPECT_EQ(url_encode("http://localhost:3000"),
            "http%3A%2F%2Flocalhost%3A3000");
}

TEST(Utility, UrlDecodeReversesUrlEncode) {
  const std::string original = "chat:read chat:edit openid & more=stuff?";
  EXPECT_EQ(url_decode(url_encode(original)), original);
}

TEST(Utility, UrlDecodeTreatsPlusAsSpace) {
  EXPECT_EQ(url_decode("chat%3Aread+chat%3Aedit"), "chat:read chat:edit");
}

TEST(Utility, UrlDecodeHandlesEmptyAndPlainStrings) {
  EXPECT_EQ(url_decode(""), "");
  EXPECT_EQ(url_decode("plain"), "plain");
}

TEST(Utility, ParseQueryStringMultipleParams) {
  auto params = parse_query_string(
      "access_token=abc123&token_type=bearer&expires_in=3600&scope=chat%3Aread+"
      "chat%3Aedit&state=deadbeef");
  ASSERT_EQ(params.count("access_token"), 1u);
  EXPECT_EQ(params["access_token"], "abc123");
  EXPECT_EQ(params["token_type"], "bearer");
  EXPECT_EQ(params["expires_in"], "3600");
  EXPECT_EQ(params["scope"], "chat:read chat:edit");
  EXPECT_EQ(params["state"], "deadbeef");
}

TEST(Utility, ParseQueryStringHandlesKeyWithNoValue) {
  auto params = parse_query_string("flag&other=value");
  ASSERT_EQ(params.count("flag"), 1u);
  EXPECT_EQ(params["flag"], "");
  EXPECT_EQ(params["other"], "value");
}

TEST(Utility, ParseQueryStringEmptyInput) {
  EXPECT_TRUE(parse_query_string("").empty());
}

TEST(Utility, ToHexProducesLeadingZeroes) {
  EXPECT_EQ(to_hex<uint8_t>(0), "00");
  EXPECT_EQ(to_hex<uint8_t>(255), "ff");
  EXPECT_EQ(to_hex<uint16_t>(1), "0001");
}

TEST(Utility, ToHexWithoutLeadingZeroes) {
  EXPECT_EQ(to_hex<uint8_t>(1, false), "1");
}
