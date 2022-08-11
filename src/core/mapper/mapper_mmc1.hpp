#ifndef NEMU_MAPPER_MMC1_HPP
#define NEMU_MAPPER_MMC1_HPP

#include "mapper.hpp"
#include "rom.hpp"

namespace nemu {

class MapperMmc1 : public Mapper {
public:
  MapperMmc1(Rom &rom) : Mapper {rom} {}

  void init() override;
  Mirror mirror() const override;

  uint32 map_prg(uint16 n) const;
  uint32 map_chr(uint16 n) const;
  uint16 map_ram(uint16 n) const;
  
  uint8 *cpu_write(uint16 n, uint8 data) override;
  const uint8 *cpu_peek(uint16 n) const override;
  uint8 *cpu_read(uint16 n) override;
  
  uint8 *ppu_write(uint16 n, uint8 data) override;
  const uint8 *ppu_peek(uint16 n) const override;
  uint8 *ppu_read(uint16 n) override;

  std::span<const uint8> pattern(uint8 n) const override;

private:
  uint8 m_control, m_buffer, m_shift;
  uint8 m_program_bank[2], m_character_bank[2];

  std::array<uint8, 0x2000> m_ram;
};

}  // namespace nemu

#endif
