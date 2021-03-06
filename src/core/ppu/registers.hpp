#ifndef NEMU_PPU_REGISTERS_HPP
#define NEMU_PPU_REGISTERS_HPP

#include "int.hpp"
#include <array>
#include <sdata.hpp>

namespace nemu {

enum PpuControl {
  PPU_NAMETABLE_ADDRESS = 0x03,
  PPU_VRAM_INCREMENT = 1 << 2,
  PPU_SPRITE_PATTERN_ADDRESS = 1 << 3,
  PPU_BACKGROUND_PATTERN_ADDRESS = 1 << 4,
  PPU_SPRITE_SIZE = 1 << 5,
  PPU_GENERATE_NMI = 1 << 7,
};

enum PpuMask {
  PPU_GREYSCALE = 1 << 0,
  PPU_LEFTMOST_BACKGROUND = 1 << 1,
  PPU_LEFTMOST_SPRITES = 1 << 2,
  PPU_SHOW_BACKGROUND = 1 << 3,
  PPU_SHOW_SPRITES = 1 << 4,
  PPU_EMPHASIZE_RED = 1 << 5,
  PPU_EMPHASIZE_GREEN = 1 << 6,
  PPU_EMPHASIZE_BLUE = 1 << 7,
};

enum PpuStatus {
  PPU_SPRITE_OVERFLOW = 1 << 5,
  PPU_SPRITE_ZERO_HIT = 1 << 6,
  PPU_VBLANK = 1 << 7,
};

struct PpuOam {
  std::array<uint8, 256> data;
  uint8 address, dma;
};

union PpuAddress {
  struct {
    uint8 coarse_x : 5;
    uint8 coarse_y : 5;
    uint8 nametable_x : 1;
    uint8 nametable_y : 1;
    uint8 fine_y : 3;
  };

  uint16 bits;
};

struct PpuRegisters {
  PpuOam oam;
  PpuAddress vram_address, temp_address;
  uint8 fine_x, control, mask, status;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

// template<>
// struct Serializer<PpuOam> : Scheme<PpuRegisters(std::array<uint8, 256>, uint8, uint8)> {
//   Map map(PpuOam &oam) {
//     return {
//       {"data", oam.data},
//       {"address", oam.address},
//       {"dma", oam.dma},
//     };
//   }
// };

// template<>
// struct Serializer<PpuRegisters> :
//   Scheme<PpuRegisters(PpuOam, uint16, uint16, uint8, uint8, uint8, uint8)> {
//   Map map(PpuRegisters &registers) {
//     return {
//       {"oam", registers.oam},
//       {"vram_address", registers.vram_address},
//       {"temp_address", registers.temp_address},
//       {"fine_x", registers.fined},
//       {"oam", registers.oam},
//       {"oam", registers.oam},
//       {"oam", registers.oam},
//       {"oam", registers.oam},
//       {"oam", registers.oam},
//     };
//   }
// };

}  // namespace sdata

#endif
