#ifndef NEMU_CPU_INTERRUPT_HPP
#define NEMU_CPU_INTERRUPT_HPP

#include "int.hpp"

namespace nemu {

struct Interrupt {
  uint8 status_mask;
  uint32 vector;
  uint32 cycles;
  bool maskable;
};

constexpr Interrupt CPU_NMI {
  0b00100000,
  0xFFFA,
  8,
  false,
};

constexpr Interrupt CPU_IRQ {
  0b00100000,
  0xFFFE,
  7,
  true,
};

constexpr Interrupt CPU_BRK {
  0b00100000,
  0xFFFE,
  {},  // Managed, by cpu instruction parser
  true,
};

}  // namespace nemu

#endif
