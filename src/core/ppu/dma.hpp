#ifndef NEMU_DMA_HPP
#define NEMU_DMA_HPP

#include "int.hpp"

namespace nemu::ppu {

struct Dma {
  uint8 page, address;
  uint8 buffer;
  uint8 w;
};

}  // namespace nemu

#endif
