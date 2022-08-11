#ifndef NEMU_ROM_HPP
#define NEMU_ROM_HPP

#include "int.hpp"
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace nemu {

enum class Mirror : uint8 {
  HORIZONTAL = 0,
  VERTICAL = 1,
  ONE_SCREEN_LO = 2,
  ONE_SCREEN_UP = 3,
  FOUR_SCREEN = 4,
};

struct RomMeta {
  // _# are ignored fields, might be used later for more advanced emulation

  char magic[4];
  uint8 prg_pages;
  uint8 chr_pages;
  uint8 mirror : 1;
  uint8 _1 : 1;
  bool has_trainer : 1;
  uint8 _2 : 1;
  uint8 mapper_lower : 4;
  uint8 _3 : 2;
  uint8 version : 2;
  uint8 mapper_upper : 4;
};

constexpr uint16 PRG_PAGE_SIZE = 0x4000;
constexpr uint16 CHR_PAGE_SIZE = 0x2000;

struct Rom {
  Rom() : meta {}, program {}, character {} {}
  Rom(std::span<uint8> data);

  RomMeta meta;
  std::span<uint8> program;
  std::span<uint8> character;
};

}  // namespace nemu

#endif
