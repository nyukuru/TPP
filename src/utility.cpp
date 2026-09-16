#include <internal/utility.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <map>
#include <string>
#include <vector>

namespace tpp {

std::vector<std::string> utility::tokenize(std::string const &in, const char *sep) {
  std::string::size_type b = 0;
  std::vector<std::string> result;

  while ((b = in.find_first_not_of(sep, b)) != std::string::npos) {
    auto e = in.find(sep, b);
    result.push_back(in.substr(b, e - b));
    b = e;
  }
  return result;
}

std::string utility::lowercase(std::string const &in) {
  std::string out {in};
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::tolower(c); });
  return out;
}

std::string utility::uppercase(std::string const &in) {
  std::string out {in};
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::toupper(c); });
  return out;
}

std::string utility::url_decode(std::string const &in) {
  std::string out;
  out.reserve(in.length());
  for (std::string::size_type i = 0; i < in.length(); ++i) {
    if (in[i] == '%' && i + 2 < in.length() && std::isxdigit((unsigned char) in[i + 1]) && std::isxdigit((unsigned char) in[i + 2])) {
      int value = 0;
      std::from_chars(in.data() + i + 1, in.data() + i + 3, value, 16);
      out.push_back((char) value);
      i += 2;
    } else if (in[i] == '+') {
      out.push_back(' ');
    } else {
      out.push_back(in[i]);
    }
  }
  return out;
}

std::string utility::url_encode(std::string const &in) {
  static constexpr char hex[] = "0123456789ABCDEF";
  std::string out;
  out.reserve(in.length() * 3);
  for (unsigned char c : in) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out.push_back((char) c);
    } else {
      out.push_back('%');
      out.push_back(hex[(c >> 4) & 0x0f]);
      out.push_back(hex[c & 0x0f]);
    }
  }
  return out;
}

std::map<std::string, std::string> utility::parse_query_string(std::string const &in) {
  std::map<std::string, std::string> out;
  for (auto const &pair : tokenize(in, "&")) {
    auto eq = pair.find('=');
    if (eq == std::string::npos) {
      out.emplace(url_decode(pair), "");
    } else {
      out.emplace(url_decode(pair.substr(0, eq)), url_decode(pair.substr(eq + 1)));
    }
  }
  return out;
}

std::string utility::base64_encode(unsigned char const *buf, unsigned int buffer_length) {
  static constexpr std::string_view to_base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  static constexpr auto push = [](std::string &dst, unsigned char b0, unsigned char b1, unsigned char b2) {
    dst.push_back(to_base64[((b0 & 0xfc) >> 2)]);
    dst.push_back(to_base64[((b0 & 0x03) << 4) + ((b1 & 0xf0) >> 4)]);
    dst.push_back(to_base64[((b1 & 0x0f) << 2) + ((b2 & 0xc0) >> 6)]);
    dst.push_back(to_base64[((b2 & 0x3f))]);
  };
  size_t ret_size = 4 * ((buffer_length + 2) / 3);// ceil(4*size/3)
  size_t i = 0;
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
      ret.push_back(to_base64[((buf[i] & 0x03) << 4) + ((buf[i + 1] & 0xf0) >> 4)]);
      ret.push_back(to_base64[((buf[i + 1] & 0x0f) << 2)]);
      ret.push_back('=');
    } else {
      ret.push_back(to_base64[((buf[i] & 0x03) << 4)]);
      ret += "==";
    }
  }
  return ret;
}

}// namespace tpp
