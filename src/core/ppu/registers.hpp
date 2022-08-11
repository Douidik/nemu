#ifndef NEMU_PPU_REGISTERS_HPP
#define NEMU_PPU_REGISTERS_HPP

#include "int.hpp"
#include <sdata.hpp>

namespace nemu::ppu {

union Scroll {
  struct {
    uint8 x, y;
  };

  uint16 bits;
};

union Control {
  struct {
    uint8 nt_x : 1;
    uint8 nt_y : 1;
    bool vram_increment : 1;
    bool spr_bank : 1;
    bool bgr_bank : 1;
    bool spr_size : 1;
    bool _ : 1;
    bool nmi : 1;
  };

  uint8 bits;
};

union Mask {
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

union Status {
  struct {
    uint8 _ : 5;

    bool spr_overflow : 1;
    bool spr_zero_hit : 1;
    bool vblank : 1;
  };

  uint8 bits;
};

struct Registers {
  uint8 w;
  uint8 buffer;
  uint8 oam_address;
  uint16 vram_address;
  Scroll scroll;
  Control control;
  Mask mask;
  Status status;
};

}  // namespace nemu::ppu

namespace sdata {

using namespace nemu;

template<>
struct Serializer<ppu::Registers> :
  Scheme<ppu::Registers(uint8, uint8, uint8, uint16, uint16, uint8, uint8, uint8)> {
  Map map(ppu::Registers &registers) {
    return {
      {"w", registers.w},
      {"buffer", registers.buffer},
      {"oam_address", registers.oam_address},
      {"vram_address", registers.vram_address},
      {"scroll", registers.scroll.bits},
      {"control", registers.control.bits},
      {"mask", registers.mask.bits},
      {"status", registers.status.bits},
    };
  }
};

}  // namespace sdata

#endif
