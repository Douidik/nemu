#ifndef NEMU_CPU_REGISTERS_HPP
#define NEMU_CPU_REGISTERS_HPP

#include "int.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <sdata.hpp>

namespace nemu {

union CpuStatus {
  uint8 bits {};

  struct {
    bool c : 1, z : 1, i : 1, d : 1, b : 1, _ : 1, v : 1, n : 1;
  };
};

struct CpuRegisters {
  CpuStatus status;
  uint8 a, x, y, sp;
  uint16 pc;
};

}  // namespace nemu

namespace fmt {
using namespace nemu;

template<>
struct formatter<CpuRegisters> {
  // Set and disabled flags representation
  constexpr static std::string_view FLAGS[2] {
    "nvubdizc",
    "NVUBDIZC",
  };

  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  auto format_register(std::string_view name, auto data, uint8 width, auto sep, F &context) const {
    return format_to(context.out(), "{}: ${:0{}X}{} ", name, data, width, sep);
  }

  template<typename F>
  auto format_status(CpuStatus status, auto sep, F &context) const {
    char output[] = "FL: ________", *iter = std::ranges::find(output, '_');

    for (uint8 n = 0; n < 8; n++) {
      *iter++ = status.bits & (0x80 >> n) ? FLAGS[1][n] : FLAGS[0][n];
    }

    return format_to(context.out(), "{}{}", output, sep);
  }

  template<typename F>
  constexpr auto format(const CpuRegisters &registers, F &context) const {
    format_to(context.out(), "{{");
    {
      format_status(registers.status, ", ", context);
      format_register("SP", registers.sp, 2, ", ", context);

      format_register("A", registers.a, 2, ", ", context);
      format_register("X", registers.x, 2, ", ", context);
      format_register("Y", registers.y, 2, ", ", context);
      format_register("ST", registers.status.bits, 2, ", ", context);
      format_register("PC", registers.pc, 4, "", context);
    }
    format_to(context.out(), "}}");

    return context.out();
  }
};

}  // namespace fmt

namespace sdata {
using namespace nemu;

template<>
struct Serializer<CpuRegisters> : Scheme<CpuRegisters(uint8, uint8, uint8, uint8, uint8, uint16)> {
  Map map(CpuRegisters &registers) {
    return {
      {"status", registers.status.bits},
      {"a", registers.a},
      {"x", registers.x},
      {"y", registers.y},
      {"sp", registers.sp},
      {"pc", registers.pc},
    };
  }
};

}  // namespace sdata

#endif
