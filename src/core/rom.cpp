#include "rom.hpp"
#include "exception.hpp"
#include "mapper/mapper.hpp"
#include <memory.h>

namespace nemu {

Rom::Rom(std::span<uint8> data) {
  memcpy(&m_meta, &data[0], sizeof(RomMeta));

  if (std::string_view {m_meta.magic, 4} != "NES\x1a") {
    throw Exception {"ROM format must be iNES standard"};
  }

  if (m_meta.version != 0) {
    throw Exception {"Nemu does not support iNES standard #{}", m_meta.version};
  }

  auto program_begin = data.begin() + (m_meta.has_trainer ? 528 : 16);

  m_program = {
    program_begin,
    program_begin + (m_meta.program_pages * 16384),
  };

  m_character = {
    m_program.end(),
    m_program.end() + (m_meta.character_pages * 8192),
  };

  m_mapper = Mapper::create(mapper_type(), m_meta.program_pages, m_meta.character_pages);
}

uint8 Rom::cpu_write(uint16 n, uint8 data) {
  return m_program[m_mapper->map_program(n)] = data;
}

uint8 Rom::cpu_read(uint16 n) {
  return m_program[m_mapper->map_program(n)];
}

uint8 Rom::ppu_read(uint16 n) {
  return m_character[m_mapper->map_character(n)];
}

}  // namespace nemu
