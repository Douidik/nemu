#ifndef NEMU_DISASM_HPP
#define NEMU_DISASM_HPP

#include "cpu/instructions.hpp"
#include <fmt/format.h>
#include <magic_enum.hpp>

namespace nemu {

struct Disasm {
  Instruction instruction;
  uint16 operand;
};

}  // namespace nemu

namespace fmt {

using namespace nemu;

template<>
struct formatter<Disasm> {
  using enum Mode;

  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format_mnemonic(Mnemonic mnemonic, F &context) const {
    return format_to(context.out(), "{} ", magic_enum::enum_name(mnemonic));
  }

  template<typename F>
  constexpr auto format_operand(Mode mode, uint16 operand, F &context) const {
    switch (mode) {
    case ACC: {
      return context.out();
    }

    case ABS: {
      return format_to(context.out(), "${:04X}", operand);
    }

    case ABX: {
      return format_to(context.out(), "${:04X},x", operand);
    }

    case ABY: {
      return format_to(context.out(), "${:04X},y", operand);
    }

    case IMM: {
      return format_to(context.out(), "#${:02X}", operand);
    }

    case IMP: {
      return context.out();
    }

    case IND: {
      return format_to(context.out(), "(${:04X})", operand);
    }

    case IDX: {
      return format_to(context.out(), "(${:02X},x)", operand);
    }

    case IDY: {
      return format_to(context.out(), "(${:02X}),y", operand);
    }

    case REL: {
      return format_to(context.out(), "${:02X}", operand);
    }

    case ZER: {
      return format_to(context.out(), "${:02X}", operand);
    }

    case ZPX: {
      return format_to(context.out(), "${:02X},x", operand);
    }

    case ZPY: {
      return format_to(context.out(), "${:02X},y", operand);
    }
    }
  }

  template<typename F>
  constexpr auto format(const Disasm &disasm, F &context) const {
    auto instruction = disasm.instruction;

    format_mnemonic<F>(instruction.mnemonic, context);
    format_operand<F>(instruction.mode, disasm.operand, context);

    return context.out();
  };
};

}  // namespace fmt

#endif
