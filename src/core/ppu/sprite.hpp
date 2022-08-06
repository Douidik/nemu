#ifndef NEMU_SPRITE_HPP
#define NEMU_SPRITE_HPP

#include "int.hpp"
#include <sdata.hpp>

namespace nemu {

struct Sprite {
  uint8 y;
  uint8 id;
  uint8 attribute;
  uint8 x;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<Sprite> : Scheme<Sprite(uint8, uint8, uint8, uint8)> {
  Map map(Sprite &sprite) {
    return {
      {"x", sprite.x},
      {"y", sprite.y},
      {"id", sprite.id},
      {"attribute", sprite.attribute},
    };
  }
};

}  // namespace sdata

#endif
