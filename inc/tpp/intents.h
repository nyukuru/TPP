#pragma once

#include <bitset>
#include <cstddef>
#include <type_traits>

#include "tpp/export.h"

namespace tpp {

/**
 * @brief EventSub subscription categories a session can set up once its
 * owning user has authenticated.
 */
enum intents : std::size_t {
  i_chat_messages,
  i_channel_points,
  i_subscriptions,
  i_follows,
  i_raids,

  /**
   * @brief Not a real intent. The number of intents above.
   */
  i_count,
};

/**
 * @brief Number of intents tpp::intent can hold.
 */
constexpr std::size_t intent_bit_count = i_count;

/**
 * @brief A set of tpp::intents.
 */
class TPP_EXPORT intent {
 protected:
  /**
   * @brief The set intents.
   */
  std::bitset<intent_bit_count> value {};

 public:
  /**
   * @brief Constructs an empty intent set.
   */
  intent() = default;

  /**
   * @brief Constructs an intent set from one or more intents.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param first an intent to set
   * @param rest further intents to set
   */
  template<typename U, typename... T>
  explicit intent(U first, T... rest) noexcept {
    value.set(first);
    (value.set(rest), ...);
  }

  /**
   * @brief Checks whether all of the given intents are set.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param values the intents to check for
   * @return true if every given intent is set
   */
  template<typename... T>
  [[nodiscard]] bool has(T... values) const noexcept {
    return (value.test(values) && ...);
  }

  /**
   * @brief Checks whether any of the given intents are set.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param values the intents to check for
   * @return true if at least one given intent is set
   */
  template<typename... T>
  [[nodiscard]] bool has_any(T... values) const noexcept {
    return (value.test(values) || ...);
  }

  /**
   * @brief Sets one or more intents.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param values the intents to set
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), intent &>
  add(T... values) noexcept {
    (value.set(values), ...);
    return *this;
  }

  /**
   * @brief Clears every intent, then sets the given intents.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param values the intents to set
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), intent &>
  set(T... values) noexcept {
    value.reset();
    (value.set(values), ...);
    return *this;
  }

  /**
   * @brief Clears one or more intents.
   * @tparam T one or more std::size_t-convertible intent positions
   * @param values the intents to clear
   * @return reference to self
   */
  template<typename... T>
  std::enable_if_t<(std::is_convertible_v<T, std::size_t> && ...), intent &>
  remove(T... values) noexcept {
    (value.reset(values), ...);
    return *this;
  }

  /**
   * @brief Union of two intent sets.
   * @return an intent set containing every intent in lhs or rhs
   */
  friend inline intent operator|(const intent &lhs,
                                 const intent &rhs) noexcept {
    intent result;
    result.value = lhs.value | rhs.value;
    return result;
  }

  /**
   * @brief Adds every intent in rhs to lhs.
   * @return reference to lhs
   */
  friend inline intent &operator|=(intent &lhs, const intent &rhs) noexcept {
    lhs.value |= rhs.value;
    return lhs;
  }

  /**
   * @brief Intersection of two intent sets.
   * @return an intent set containing only the intents present in both lhs
   * and rhs
   */
  friend inline intent operator&(const intent &lhs,
                                 const intent &rhs) noexcept {
    intent result;
    result.value = lhs.value & rhs.value;
    return result;
  }

  /**
   * @brief Clears every intent in lhs that is not also set in rhs.
   * @return reference to lhs
   */
  friend inline intent &operator&=(intent &lhs, const intent &rhs) noexcept {
    lhs.value &= rhs.value;
    return lhs;
  }
};

/**
 * @brief Union of two intents.
 * @return an intent set containing lhs and rhs
 */
inline intent operator|(intents lhs, intents rhs) noexcept {
  return intent(lhs, rhs);
}

/**
 * @brief Adds an intent to an intent set.
 * @return an intent set containing lhs and rhs
 */
inline intent operator|(const intent &lhs, intents rhs) noexcept {
  return lhs | intent(rhs);
}

/**
 * @brief Adds an intent to an intent set.
 * @return an intent set containing lhs and rhs
 */
inline intent operator|(intents lhs, const intent &rhs) noexcept {
  return intent(lhs) | rhs;
}

/**
 * @brief Adds an intent to an intent set.
 * @return reference to lhs
 */
inline intent &operator|=(intent &lhs, intents rhs) noexcept {
  return lhs |= intent(rhs);
}

/**
 * @brief Intersection of an intent set with a single intent.
 * @return an intent set containing rhs if lhs has it set, otherwise empty
 */
inline intent operator&(const intent &lhs, intents rhs) noexcept {
  return lhs & intent(rhs);
}

/**
 * @brief Intersection of an intent set with a single intent.
 * @return an intent set containing lhs if rhs has it set, otherwise empty
 */
inline intent operator&(intents lhs, const intent &rhs) noexcept {
  return intent(lhs) & rhs;
}

/**
 * @brief Intersection of two intents.
 * @return an intent set containing lhs if lhs equals rhs, otherwise empty
 */
inline intent operator&(intents lhs, intents rhs) noexcept {
  return intent(lhs) & intent(rhs);
}

/**
 * @brief Clears lhs's intent unless it equals rhs.
 * @return reference to lhs
 */
inline intent &operator&=(intent &lhs, intents rhs) noexcept {
  return lhs &= intent(rhs);
}

}// namespace tpp
