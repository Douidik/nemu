#ifndef NEMU_MAPPER_HPP
#define NEMU_MAPPER_HPP

#include "int.hpp"
#include <memory>

namespace nemu {

class Mapper {
public:
  Mapper(uint8 program_pages, uint8 character_pages) :
    m_program_pages {program_pages},
    m_character_pages {character_pages} {}

  static std::shared_ptr<Mapper> create(uint8 type, uint8 program_pages, uint8 character_pages);

  virtual uint32 map_program_peek(uint32 n) const = 0;
  virtual uint32 map_character_peek(uint32 n) const = 0;

  virtual uint32 map_program(uint32 n) {
    return {};
  }

  virtual uint32 map_character(uint32 n) {
    return {};
  }

  virtual uint32 map_program_read(uint32 n) {
    return map_program(n);
  }

  virtual uint32 map_program_write(uint32 n) {
    return map_program(n);
  }

  virtual uint32 map_character_read(uint32 n) {
    return map_character(n);
  }

  virtual uint32 map_character_write(uint32 n) {
    return map_character(n);
  }

protected:
  uint8 m_program_pages, m_character_pages;
};

}  // namespace nemu

#endif
