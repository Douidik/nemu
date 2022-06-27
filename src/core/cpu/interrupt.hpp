#ifndef NEMU_CPU_INTERRUPT_HPP
#define NEMU_CPU_INTERRUPT_HPP

#include "int.hpp"

namespace nemu {

struct Interrupt {
  enum : uint8 {
    PUSH_PC = 1 << 0,
    PUSH_STATUS = 1 << 1,
  };

  uint8 status_mask;
  uint32 vector;
  uint32 cycles;
  bool maskable;
  uint8 push;
};

constexpr Interrupt CPU_NMI {
  0b00100000,
  0xFFFA,
  8,
  false,
  Interrupt::PUSH_PC | Interrupt::PUSH_STATUS,
};

constexpr Interrupt CPU_IRQ {
  0b00100000,
  0xFFFE,
  7,
  true,
  Interrupt::PUSH_PC | Interrupt::PUSH_STATUS,
};

constexpr Interrupt CPU_BRK {
  0b00100000,
  0xFFFE,
  {},  // Managed, by cpu instruction parser
  true,
  Interrupt::PUSH_PC | Interrupt::PUSH_STATUS,
};

constexpr Interrupt CPU_RST {
  0b00110000,
  0xFFFC,
  0,
  false,
  {},
};

}  // namespace nemu

#endif
