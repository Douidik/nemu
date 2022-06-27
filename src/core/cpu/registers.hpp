#ifndef NEMU_CPU_REGISTERS_HPP
#define NEMU_CPU_REGISTERS_HPP

#include "int.hpp"
#include <fmt/format.h>

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

template<>
struct fmt::formatter<nemu::CpuRegisters> {
  constexpr static std::string_view FLAGS = "NVUBDIZC";

  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  auto
  format_register(std::string_view name, nemu::uint16 reg, nemu::uint8 width, F &context) const {
    return format_to(context.out(), "| {}: ${:0{}X} |", name, reg, width);
  }

  // Format status registers: CZIDBUVN, '_' means disabled
  template<typename F>
  auto format_status(nemu::CpuStatus status, F &context) const {
    char output[] = "| FL: ________ |", *iter = &output[6];

    for (nemu::uint8 n = 0; n < FLAGS.size(); n++) {
      *iter++ = status.bits & (0x80 >> n) ? FLAGS[n] : '_';
    }

    return format_to(context.out(), "{}", output);
  }

  template<typename F>
  constexpr auto format(const nemu::CpuRegisters &registers, F &context) const {
    format_register("A", registers.a, 2, context);
    format_register("X", registers.x, 2, context);
    format_register("Y", registers.y, 2, context);
    format_register("SP", registers.sp, 2, context);
    format_register("ST", registers.status.bits, 2, context);
    format_status(registers.status, context);
    format_register("PC", registers.pc, 4, context);

    return context.out();
  }
};

#endif
