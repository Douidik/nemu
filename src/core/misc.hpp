#ifndef NEMU_MISC_HPP
#define NEMU_MISC_HPP

#include "exception.hpp"
#include <algorithm>
#include <optional>

namespace nemu {

template<typename F, typename R = std::invoke_result_t<F>>
constexpr auto invoke_default_value() -> R {
  if constexpr (std::same_as<R, void>) {
    return;
  } else {
    return R {};
  }
}

template<std::integral T, std::integral N>
constexpr auto pow(T x, N n) -> T {
  if (!n) {
    return 1;
  }

  if (n > 0) {
    return x * pow(x, n - 1);
  }

  if (n < 0) {
    return 1 / pow(x, -n);
  }

  return {};
};

template<std::integral T>
constexpr auto parse_int(const char *iter, const char *end) -> T {
  size_t distance = std::distance(iter, end);

  if (distance < 1) {
    return {};
  }

  if (*iter < '0' || *iter > '9') {
    throw Exception {"Unexpected character when parsing number: '{}'", *iter};
  }

  return ((*iter - '0') * pow(10, distance - 1)) + parse_int<T>(iter + 1, end);
}

}  // namespace nemu

#endif
