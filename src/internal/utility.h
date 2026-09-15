#pragma once

#include <charconv>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace tpp::utility {
std::vector<std::string> tokenize(std::string const &in,
                                  const char        *sep = "\r\n");

/**
 * @brief Convert a numeric value to hex
 *
 * @tparam T numeric type
 * @param i numeric value
 * @param leading_zeroes set to false if you don't want the leading zeroes in
 * the output
 * @return std::string value in hex, the length will be 2* the raw size of the
 * type
 */
template<typename T>
std::string to_hex(T i, bool leading_zeroes = true) {
  char   str[26] = {0};
  size_t size    = sizeof(T) * 2;
  std::to_chars(std::begin(str), std::end(str), i, 16);
  std::string out {str};
  if (leading_zeroes && out.length() < size) {
    out.insert(out.begin(), size - out.length(), '0');
  }
  return out;
}

/**
 * @brief Lowercase a string
 * @param in input string
 * @return lowercased copy of in
 */
std::string lowercase(std::string const &in);

/**
 * @brief Uppercase a string
 * @param in input string
 * @return uppercased copy of in
 */
std::string uppercase(std::string const &in);

/**
 * @brief Percent-decode a URL component, also treating '+' as a space
 * (form-encoding convention).
 * @param in encoded input
 * @return decoded string
 */
std::string url_decode(std::string const &in);

/**
 * @brief Percent-encode a string for safe use inside a URL query component.
 * @param in raw input
 * @return encoded string
 */
std::string url_encode(std::string const &in);

/**
 * @brief Parse an "a=b&c=d" style query/fragment string into a map of
 * decoded key/value pairs.
 * @param in query string, without the leading '?' or '#'
 * @return decoded key/value pairs
 */
std::map<std::string, std::string> parse_query_string(std::string const &in);

std::string base64_encode(unsigned char const *buf, unsigned int buffer_length);
}// namespace tpp::utility

