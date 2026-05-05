#pragma once

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
std::string to_hex(T i, bool leading_zeroes = true);

std::string lower(std::string const &in);

std::string base64_encode(unsigned char const *buf, unsigned int buffer_length);
}// namespace tpp::utility

