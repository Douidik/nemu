#ifndef NEMU_CPU_REGISTERS_HPP
#define NEMU_CPU_REGISTERS_HPP

#include "int.hpp"

namespace nemu {

union CpuStatus {
  struct {
    uint8 c : 1, z : 1, i : 1, d : 1, b : 1, _ : 1, v : 1, n : 1;
  };

  uint8 raw {};
};

struct CpuRegisters {
  CpuStatus status;
  uint8 a, x, y, sp;
  uint16 pc;
};

}  // namespace nemu

#endif
