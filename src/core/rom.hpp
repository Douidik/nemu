#ifndef NEMU_ROM_HPP
#define NEMU_ROM_HPP

#include "int.hpp"
#include <memory>
#include <span>

namespace nemu {

enum class Mirror : uint8 {
  HORIZONTAL = 0,
  VERTICAL = 1,
};

struct RomMeta {
  // _# are ignored fields, might be used for advanced emulation

  char magic[4];
  uint8 program_pages;
  uint8 character_pages;
  Mirror mirror : 1;
  uint8 _1 : 1;
  bool has_trainer : 1;
  uint8 _2 : 1;
  uint8 mapper_lower : 4;
  uint8 _3 : 2;
  uint8 version : 2;
  uint8 mapper_upper : 4;
};

class Rom {
public:
  explicit Rom(std::span<uint8> data);

  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_read(uint16 n);

  uint8 ppu_write(uint16 n, uint8 data);
  uint8 ppu_read(uint16 n);

  inline const auto &meta() const {
    return m_meta;
  }

  inline uint8 mapper_type() const {
    return (m_meta.mapper_upper & 0xFF00) | (m_meta.mapper_lower >> 4);
  }

  inline Mirror mirror() const {
    return m_meta.mirror;
  }

private:
  RomMeta m_meta;
  std::span<uint8> m_program;
  std::span<uint8> m_character;
  std::shared_ptr<class Mapper> m_mapper;
};

}  // namespace nemu

#endif
