#ifndef NEMU_NES_HPP
#define NEMU_NES_HPP

#include "bus.hpp"
#include "ppu/ppu.hpp"
#include "rom.hpp"
#include "gamepad.hpp"

namespace nemu {

class Nes : public Bus {
public:
  Nes(Rom &rom);

  void init() override;
  void tick() override;

  uint8 cpu_write(uint16 n, uint8 data) override;
  uint8 cpu_read(uint16 n) override;
  uint8 ppu_read(uint16 n);

  Gamepad &gamepad(uint8 n) {
    return m_gamepads[n];
  }
  
  Ppu &ppu() {
    return m_ppu;
  }

  Rom &rom() {
    return m_rom;
  }

private:
  Ppu m_ppu;
  Rom &m_rom;
  Gamepad m_gamepads[2];
};

}  // namespace nemu

#endif
