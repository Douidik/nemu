#ifndef NEMU_MAPPER_NROM_HPP
#define NEMU_MAPPER_NROM_HPP

#include "mapper.hpp"

namespace nemu {

class MapperNRom : public Mapper {
public:
  using Mapper::Mapper;

  inline uint32 map_program(uint32 n) override {
    return (n - 0x8000) % 0x4000;
  }

  inline uint32 map_character(uint32 n) override {
    return n;
  }
};

}  // namespace nemu

#endif
