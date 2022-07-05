#ifndef NEMU_CANVAS_HPP
#define NEMU_CANVAS_HPP

#include "int.hpp"
#include <array>

namespace nemu {

enum CanvasSize : uint16 {
  CANVAS_W = 256,
  CANVAS_H = 240,
};

struct Canvas {
  std::array<std::array<uint8, CANVAS_H>, CANVAS_W> buffer {};
};

}  // namespace nemu

#endif
