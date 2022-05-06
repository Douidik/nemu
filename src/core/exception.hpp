#ifndef NEMU_EXCEPTION_HPP
#define NEMU_EXCEPTION_HPP

#include <exception>
#include <fmt/format.h>
#include <string>
#include <string_view>

namespace nemu {

class Exception : std::exception {
  constexpr static std::string_view PATTERN = "[NEMU exception raised]: {}";

public:
  Exception(auto description, auto... args) :
    m_buffer {fmt::format(PATTERN, fmt::vformat(description, fmt::make_format_args(args)...))} {}

  Exception(auto description) : m_buffer {fmt::format(PATTERN, description)} {}

  inline const char *what() const noexcept override {
    return m_buffer.data();
  }

private:
  std::string m_buffer {};
};

}  // namespace nemu

#endif
