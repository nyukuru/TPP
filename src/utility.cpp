#include <internal/utility.h>

#include <algorithm>
#include <charconv>
#include <string>
#include <vector>

namespace tpp::utility {

std::vector<std::string> tokenize(std::string const &in, const char *sep) {
  std::string::size_type   b = 0;
  std::vector<std::string> result;

  while ((b = in.find_first_not_of(sep, b)) != std::string::npos) {
    auto e = in.find(sep, b);
    result.push_back(in.substr(b, e - b));
    b = e;
  }
  return result;
}

std::string lower(std::string const &in) {
  std::transform(in.begin(), in.end(), in.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

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
std::string to_hex(T i, bool leading_zeroes) {
  char   str[26] = {0};
  size_t size    = sizeof(T) * 2;
  std::to_chars(std::begin(str), std::end(str), i, 16);
  std::string out {str};
  if (leading_zeroes && out.length() < size) {
    out.insert(out.begin(), size - out.length(), '0');
  }
  return out;
}

std::string base64_encode(unsigned char const *buf,
                          unsigned int         buffer_length) {
  /* Quick and dirty base64 encode */
  static constexpr std::string_view to_base64 =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  static constexpr auto push = [](std::string &dst, unsigned char b0,
                                  unsigned char b1, unsigned char b2) {
    dst.push_back(to_base64[((b0 & 0xfc) >> 2)]);
    dst.push_back(to_base64[((b0 & 0x03) << 4) + ((b1 & 0xf0) >> 4)]);
    dst.push_back(to_base64[((b1 & 0x0f) << 2) + ((b2 & 0xc0) >> 6)]);
    dst.push_back(to_base64[((b2 & 0x3f))]);
  };
  size_t      ret_size = 4 * ((buffer_length + 2) / 3);// ceil(4*size/3)
  size_t      i        = 0;
  std::string ret;

  ret.reserve(ret_size);

  if (buffer_length > 2) {//    vvvvv avoid unsigned overflow
    while (i < buffer_length - 2) {
      push(ret, buf[i], buf[i + 1], buf[i + 2]);
      i += 3;
    }
  }
  size_t left = buffer_length - i;
  if (left >= 1) {// handle non-multiple of 3s, pad the end with =
    ret.push_back(to_base64[((buf[i] & 0xfc) >> 2)]);
    if (left >= 2) {
      ret.push_back(
          to_base64[((buf[i] & 0x03) << 4) + ((buf[i + 1] & 0xf0) >> 4)]);
      ret.push_back(to_base64[((buf[i + 1] & 0x0f) << 2)]);
      ret.push_back('=');
    } else {
      ret.push_back(to_base64[((buf[i] & 0x03) << 4)]);
      ret += "==";
    }
  }
  return ret;
}

}// namespace tpp::utility

