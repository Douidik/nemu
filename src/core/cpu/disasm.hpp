#ifndef NEMU_DISASM_HPP
#define NEMU_DISASM_HPP

#include "cpu/instructions.hpp"
#include "cpu/registers.hpp"
#include "exception.hpp"
#include <charconv>
#include <concepts>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <variant>
#include <vector>

namespace nemu {

struct Disasm {
  union Operation {
    struct Load {};

    struct Store {};

    struct Transfer {
      struct Operand {
        std::string_view name;
        uint8 data;
      } source, destination;
    };

    struct Stack {
      std::string_view description;
      uint8 stack_pointer;
    };

    struct Increment {
      std::string_view description;
      uint8 data;
    };

    struct BinaryOp {
      std::string_view description;
      uint8 a, b;
    };

    struct Shift {
      std::string_view description;
      uint8 data;
    };

    struct Flag {
      std::string_view description;
    };

    struct Comparison {
      uint8 a, b;
    };

    struct Conditional {
      std::string_view description;
      bool condition;
    };

    struct Jump {};

    struct Interrupt {};

    struct OtherOp {};

    Load load;
    Store store;
    Transfer transfer;
    Stack stack;
    Increment increment;
    BinaryOp binary_op;
    Shift shift;
    Flag flag;
    Comparison comparison;
    Conditional conditional;
    Jump jump;
    Interrupt interrupt;
    OtherOp other_op;
  };

  union Fetch {
    struct Acc {
      uint8 data;
    };

    struct Abs {
      uint16 address;
      uint8 data;
    };

    struct Abx {
      uint16 destination, address;
      uint8 offset, data;
    };

    struct Aby {
      uint16 destination, address;
      uint8 offset, data;
    };

    struct Imm {
      uint8 data;
    };

    struct Imp {};

    struct Ind {
      uint16 destination, address, data;
    };

    struct Idx {
      uint16 destination;
      uint8 address, offset, data;
    };

    struct Idy {
      uint16 destination;
      uint8 address, offset, data;
    };

    struct Rel {
      uint16 destination;
      uint16 start;
      int8 offset;
    };

    struct Zer {
      uint8 address, data;
    };

    struct Zpx {
      uint8 destination, address, offset, data;
    };

    struct Zpy {
      uint8 destination, address, offset, data;
    };

    Acc acc;
    Abs abs;
    Abx abx;
    Aby aby;
    Imm imm;
    Imp imp;
    Ind ind;
    Idx idx;
    Idy idy;
    Rel rel;
    Zer zer;
    Zpx zpx;
    Zpy zpy;
  };

  struct Bytes : std::vector<uint8> {
    using vector::vector;
  };

  Instruction instruction;
  Operation operation;
  Fetch fetch;
  Bytes bytes;
  CpuRegisters registers;
};

}  // namespace nemu

namespace fmt {
using namespace nemu;

template<>
struct formatter<Disasm::Fetch> {
  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format(const Disasm::Fetch fetch, F &context) const {
    // Fetch variant visitor
    auto fetch_visitor = [&context]<typename T>(T fetch) -> F & {
      if constexpr (std::same_as<T, Disasm::Acc>) {
        format_to(context.out(), "A [=#${:02X}]", fetch.data);
      }

      if constexpr (std::same_as<T, Disasm::Abs>) {
        format_to(context.out(), "${:04X} [=#${:02X}]", fetch.address, fetch.data);
      }

      if constexpr (std::same_as<T, Disasm::Abx>) {
        format_to(
          context.out(),
          "#${:04X},x [=#${:02X}, x:#${:02X}, d:${:04X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Aby>) {
        format_to(
          context.out(),
          "#${:04X},x [=#${:02X}, y:#${:02X}, d:${:04X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Imm>) {
        format_to(context.out(), "#${:02X}", fetch.data);
      }

      if constexpr (std::same_as<T, Disasm::Ind>) {
        format_to(
          context.out(),
          "(${:04X}) [=#${:02X}, d:${:04X}]",
          fetch.address,
          fetch.data,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Idx>) {
        format_to(
          context.out(),
          "(${:04X}) [=#${:02X}, x:#${:02X}, d:${:04X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Idy>) {
        format_to(
          context.out(),
          "(${:04X}) [=#${:02X}, y:#${:02X}, d:${:04X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Rel>) {
        format_to(context.out(), "${:04X}", fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Zer>) {
        format_to(context.out(), "${:02X} [=#${:02X}]", fetch.address, fetch.data);
      }

      if constexpr (std::same_as<T, Disasm::Zpx>) {
        format_to(
          context.out(),
          "${:02X} [=#${:02X}, x:#${:02X}, d:${:02X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      if constexpr (std::same_as<T, Disasm::Zpy>) {
        format_to(
          context.out(),
          "${:02X} [=#${:02X}, y:#${:02X}, d:${:02X}]",
          fetch.address,
          fetch.data,
          fetch.offset,
          fetch.destination);
      }

      return context;
    };

    return std::visit(fetch_visitor, fetch).out();
  }
};

template<>
struct formatter<Disasm::Operation> {
  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format(const Disasm::Operation operation, F &context) const {
    // Operation variant visitor
    auto operation_visitor = [&context]<typename T>(T operation) -> F & {
      if constexpr (std::same_as<T, Disasm::Transfer>) {
        auto src = operation.source;
        auto dst = operation.destination;

        format_to(context.out(), "{}:{:02X} => {}:{:02X}", src.name, src.data, dst.name, dst.data);
      }

      if constexpr (std::same_as<T, Disasm::Stack>) {
        format_to(
          context.out(),
          "{}",
          operation.description,
          "index"_a = (0xFD - operation.stack_pointer));
      }

      if constexpr (std::same_as<T, Disasm::Increment>) {
        format_to(context.out(), "{}", operation.description, "data"_a = operation.data);
      }

      if constexpr (std::same_as<T, Disasm::BinaryOp>) {
        format_to(
          context.out(),
          "{}",
          operation.description,
          "a"_a = operation.a,
          "b"_a = operation.b);
      }

      if constexpr (std::same_as<T, Disasm::Shift>) {
        format_to(context.out(), "{}", operation.description, "data"_a = operation.data);
      }

      if constexpr (std::same_as<T, Disasm::Flag>) {
        format_to(context.out(), "{}", operation.description);
      }

      if constexpr (std::same_as<T, Disasm::Comparison>) {
        // constexpr auto format_comparison = [&](char op, char result, std::string_view sep) {
        //   format_to(context.out(), "{}: {} {} {}{}", result, operation.a, op, operation.b, sep);
        // };

        // constexpr auto do_comparison = []<auto expression>(char flags[2])->char {
        //   return flags[expression];
        // };

        // format_comparison('<', do_comparison<(operation.a < operation.b)>("nN"), ", ");
        // format_comparison('=', do_comparison<(operation.a == operation.b)>("zZ"), ", ");
        // format_comparison('>', do_comparison<(operation.a > operation.b)>("cC"), ", ");
      }

      if constexpr (std::same_as<T, Disasm::Conditional>) {
        format_to(context.out(), "{} ? {}", operation.description, operation.condition ? "y" : "n");
      }

      return context;
    };

    return std::visit(operation_visitor, operation).out();
  }
};

template<>
struct formatter<Disasm::Bytes> {
  template<typename P>
  constexpr auto parse(P &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format_byte(const Disasm::Bytes &bytes, size_t i, F &context) const {
    if (i > bytes.size()) {
      return format_to(context.out(), "__");
    } else {
      return format_to(context.out(), "{:02X}", bytes[i]);
    }
  }

  template<typename F>
  constexpr auto format(const Disasm::Bytes &bytes, F &context) const {
    format_to(context.out(), "<");

    for (size_t i = 0; i < Instruction::max_size(); i++) {
      format_byte(bytes, i, context);

      // Seperate each bytes with '|'
      if (i < Instruction::max_size() - 1) {
        format_to(context.out(), "|");
      }
    }

    return format_to(context.out(), ">");
  }
};

template<>
struct formatter<Disasm> {
  template<typename P>
  constexpr auto parse(P &context) {
    auto end = std::ranges::find(context, '}');
    auto width_specifier = std::ranges::find(context, ':');

    if (width_specifier != context.end()) {
      std::string_view width_source {width_specifier + 1, end};
      auto [_, status] = std::from_chars(width_source.begin(), width_source.end(), *m_width);

      if (status != std::errc {}) {
        throw Exception {"Width specifier parsing failed from: '{}'", width_source};
      }
    }

    return end;
  }

  template<typename F>
  constexpr auto format(const Disasm &disasm, F &context) const {
    size_t size {
      formatted_size("{} {} {}", disasm.registers, disasm.bytes, disasm.fetch),
    };

    auto status {
      format_to_n(context.out(), size, "{} {} {}", disasm.registers, disasm.bytes, disasm.fetch),
    };

    if (m_width.has_value()) {
      format_to(context.out(), "| {:>{}}", disasm.operation, *m_width - size);
    } else {
      format_to(context.out(), "| {}", disasm.operation);
    }

    return context.out();
  }

  std::optional<size_t> m_width {};
};

}  // namespace fmt

#endif
