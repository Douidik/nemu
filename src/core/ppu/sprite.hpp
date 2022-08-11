#ifndef NEMU_SPRITE_HPP
#define NEMU_SPRITE_HPP

#include "int.hpp"
#include <span>

namespace nemu::ppu {

union SpriteAttribute {
  uint8 bits;

  struct {
    uint8 color : 2;
    uint8 _ : 3;
    uint8 priority : 1;
    uint8 flip : 2;
  };
};

struct Sprite {
  constexpr static Sprite from_span(std::span<const uint8> span) {
    return {
      {span[3], span[0]},
      span[1],
      span[2],
    };
  }

  uint8 position[2];
  uint8 index;
  SpriteAttribute ab;
};

}  // namespace nemu::ppu

#endif
