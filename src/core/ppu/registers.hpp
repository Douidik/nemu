#ifndef NEMU_PPU_REGISTERS_HPP
#define NEMU_PPU_REGISTERS_HPP

#include "int.hpp"
#include <sdata.hpp>

namespace nemu {

union PpuScroll {
  struct {
    uint8 x, y;
  };

  uint16 bits;
};

struct PpuLatch {
  inline bool use() {
    return value = !value;
  }

  inline bool reset() {
    return value = false;
  }

  bool value;
};

union PpuControl {
  struct {
    uint8 nt_address : 3;
    bool vram_increment : 1;
    bool spr_address : 1;
    bool bgr_address : 1;
    bool spr_size : 1;
    bool nmi : 1;
  };

  uint8 bits;
};

union PpuMask {
  struct {
    bool greyscale : 1;
    bool bgr_leftmost : 1;
    bool spr_leftmost : 1;
    bool bgr_show : 1;
    bool spr_show : 1;
    bool red_emphasize : 1;
    bool green_emphasize : 1;
    bool blue_emphasize : 1;
  };

  uint8 bits;
};

union PpuStatus {
  struct {
    uint8 _ : 5;

    bool spr_overflow : 1;
    bool spr_zero_hit : 1;
    bool vblank : 1;
  };

  uint8 bits;
};

struct PpuRegisters {
  uint16 address;
  PpuScroll scroll;
  PpuLatch latch;
  PpuControl control;
  PpuMask mask;
  PpuStatus status;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<PpuRegisters> : Scheme<PpuRegisters(uint16, uint16, bool, uint8, uint8, uint8)> {
  Map map(PpuRegisters &registers) {
    return {
      {"address", registers.address},
      {"scroll", registers.scroll.bits},
      {"latch", registers.latch.value},
      {"control", registers.control.bits},
      {"mask", registers.mask.bits},
      {"status", registers.status.bits},
    };
  }
};

}  // namespace sdata

#endif
