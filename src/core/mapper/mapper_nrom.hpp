#ifndef NEMU_MAPPER_NROM_HPP
#define NEMU_MAPPER_NROM_HPP

#include "mapper.hpp"
#include "rom.hpp"

namespace nemu {

class MapperNRom : public Mapper {
public:
  MapperNRom(Rom &rom) : Mapper {rom} {}

  uint16 map_prg(uint16 n) const {
    return n & (m_rom.meta.prg_pages * PRG_PAGE_SIZE - 1);
  }

  uint16 map_chr(uint16 n) const {
    return n & CHR_PAGE_SIZE;
  }

  Mirror mirror() const override {
    return static_cast<Mirror>(m_rom.meta.mirror);
  }

  uint8 *cpu_write(uint16 n, uint8 data) override {
    return n > 0x7FFF ? &(m_rom.program[map_prg(n)] = data) : nullptr;
  }

  const uint8 *cpu_peek(uint16 n) const override {
    return n > 0x7FFF ? &(m_rom.program[map_prg(n)]) : nullptr;
  }

  uint8 *cpu_read(uint16 n) override {
    return n > 0x7FFF ? &(m_rom.program[map_prg(n)]) : nullptr;
  }

  uint8 *ppu_write(uint16 n, uint8 data) override {
    return n < 0x2000 ? &(m_rom.program[map_chr(n)] = data) : nullptr;
  }

  const uint8 *ppu_peek(uint16 n) const override {
    return n < 0x2000 ? &(m_rom.program[map_chr(n)]) : nullptr;
  }

  uint8 *ppu_read(uint16 n) override {
    return n < 0x2000 ? &(m_rom.program[map_chr(n)]) : nullptr;
  }

  std::span<const uint8> pattern(uint8 n) const override {
    return {&m_rom.character[n * 0x1000], 0x1000};
  }
};

}  // namespace nemu

#endif
