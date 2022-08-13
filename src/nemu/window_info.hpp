#ifndef NEMU_WINDOW_INFO_HPP
#define NEMU_WINDOW_INFO_HPP

#include "int.hpp"
#include <sdata.hpp>

namespace nemu {

struct WindowInfo {
  uint32 width, height, options;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<WindowInfo> : Scheme<WindowInfo(uint32, uint32, uint32)> {
  Map map(WindowInfo &info) {
    return Map {
      {"width", info.width},
      {"height", info.height},
      {"options", info.options},
    };
  }
};

}  // namespace sdata

#endif
