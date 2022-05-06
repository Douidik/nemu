#ifndef NEMU_NES_HPP
#define NEMU_NES_HPP

#include "bus.hpp"
#include "ppu/ppu.hpp"
#include "rom.hpp"

namespace nemu {

class Nes : public Bus {
public:
  Nes(Rom &rom);

  void tick() override;

  uint8 cpu_write(uint16 n, uint8 data) override;
  uint8 cpu_read(uint16 n) override;

  uint8 ppu_write(uint16 n, uint8 data) override;
  uint8 ppu_read(uint16 n) override;

  uint8 apu_write(uint16 n, uint8 data) override;
  uint8 apu_read(uint16 n) override;

  const Ppu &ppu() const {
    return m_ppu;
  }

  const Rom &rom() const {
    return m_rom;
  }

private:
  Ppu m_ppu;
  Rom &m_rom;
};

}  // namespace nemu

#endif
