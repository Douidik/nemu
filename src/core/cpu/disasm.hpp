#ifndef NEMU_DISASM_HPP
#define NEMU_DISASM_HPP

#include "exception.hpp"
#include "instructions.hpp"
#include "misc.hpp"
#include "registers.hpp"
#include <algorithm>
#include <charconv>
#include <fmt/format.h>
#include <vector>

namespace nemu {

struct Disasm {
  struct Bytes : std::vector<uint8> {
    using vector::vector;
  };

  struct Fetch {
    Mode mode;

    std::string_view description;
    uint16 address;
    uint16 destination;
    uint8 offset;
    uint8 data;
  };

  struct Operation {
    Mnemonic mnemonic;

    std::string_view description;
    uint8 operands[2];
  };

  CpuRegisters registers;
  Bytes bytes;
  Fetch fetch;
  Operation operation;
};

}  // namespace nemu

namespace fmt {

using namespace nemu;
namespace ranges = std::ranges;

template<>
struct formatter<Disasm::Bytes> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template<typename F>
  auto format_byte(const Disasm::Bytes &bytes, size_t i, F &context) const {
    if (i > bytes.size() - 1) {
      return format_to(context.out(), "__");
    } else {
      return format_to(context.out(), "{:02X}", bytes[i]);
    }
  }

  template<typename F>
  constexpr auto format(const Disasm::Bytes &bytes, F &context) const {
    for (size_t i = 0; i < Instruction::max_size(); i++) {
      format_byte(bytes, i, context);

      // Seperate each bytes with '|'
      if (i < Instruction::max_size() - 1) {
        format_to(context.out(), "|");
      }
    }

    return context.out();
  }
};

template<>
struct formatter<Disasm::Fetch> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format(const Disasm::Fetch &fetch, F &context) const {
    const auto &[mode, desc, address, dst, offset, data] = fetch;

    if (mode & (IND)) {
      return format_to(context.out(), fmt::runtime(desc), address, data, dst);
    }

    if (mode & (REL)) {
      return format_to(context.out(), fmt::runtime(desc), dst);
    }

    if (mode & (ACC | IMM)) {
      return format_to(context.out(), fmt::runtime(desc), data);
    }

    if (mode & (ZER | ABS)) {
      return format_to(context.out(), fmt::runtime(desc), address, data);
    }

    if (mode & (ABX | ABY | IDX | IDY | ZPX | ZPY)) {
      return format_to(context.out(), fmt::runtime(desc), address, data, offset, dst);
    }

    return format_to(context.out(), fmt::runtime(desc));
  }
};

template<>
struct formatter<Disasm::Operation> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format(const Disasm::Operation &operation, F &context) {
    const auto &[mnemonic, desc, operands] = operation;

    if (mnemonic & (TRANSFER | BINARY_OP)) {
      return format_to(context.out(), fmt::runtime(desc), operands[0], operands[1]);
    }

    if (mnemonic & (LOAD | STORE | STACK | INCREMENT | SHIFT)) {
      return format_to(context.out(), fmt::runtime(desc), operands[0]);
    }

    if (mnemonic & (CONDITIONAL)) {
      return format_to(context.out(), fmt::runtime(desc), static_cast<bool>(operands[0]));
    }

    if (mnemonic & (COMPARISON)) {
      const char &n = "nN"[operands[0] > operands[1]];
      const char &c = "cC"[operands[0] < operands[1]];
      const char &z = "zZ"[operands[0] == operands[1]];

      return format_to(context.out(), fmt::runtime(desc), operands[0], operands[1], n, c, z);
    }

    return format_to(context.out(), fmt::runtime(desc));
  }
};

template<>
struct formatter<Disasm> {
  constexpr auto parse(format_parse_context &context) {
    auto end = ranges::find(context, '}');

    auto width_begin = ranges::find(context, '(');
    auto width_close = ranges::find(context, ')');

    if (width_begin != context.end() && width_close != context.end()) {
      width = parse_int<int32>(width_begin + 1, width_close);
    }

    return end;
  }

  template<typename F>
  constexpr auto format(const Disasm &disasm, F &context) const {
    const auto &[registers, bytes, fetch, operation] = disasm;
    Mnemonic mnemonic = operation.mnemonic;

    size_t padding {
      formatted_size("{} |{}| {} {}", registers, bytes, mnemonic, fetch),
    };

    auto status {
      format_to_n(context.out(), padding, "{} |{}| {} {}", registers, bytes, mnemonic, fetch),
    };

    size_t operation_size = formatted_size("{}", operation);

    if (operation_size > 0) {
      if (width.has_value()) {
        // Anchor the operation description to the right
        format_to(context.out(), "{: >{}}", "> ", *width - padding - operation_size - 1);
      }

      format_to_n(context.out(), operation_size + 1, " {}", operation);
    }

    return context.out();
  }

  std::optional<int32> width {};
};

}  // namespace fmt

#endif
