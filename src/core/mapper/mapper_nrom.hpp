#ifndef NEMU_MAPPER_NROM_HPP
#define NEMU_MAPPER_NROM_HPP

#include "mapper.hpp"

namespace nemu {

class MapperNRom : public Mapper {
public:
  using Mapper::Mapper;

  inline uint32 map_program(uint32 n) override {
    // return n - 0x8000;  // return (n - 0x8000) % 0x4000;

    return n & (m_program_pages > 1 ? 0x7FFF : 0x3FFF);
  }

  inline uint32 map_character(uint32 n) override {
    return n;
  }
};

}  // namespace nemu

#endif
