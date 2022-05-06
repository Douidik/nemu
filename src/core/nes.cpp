#include "nes.hpp"

namespace nemu {

Nes::Nes(Rom &rom) : m_ppu {this}, m_rom {rom} {
  m_cpu.init();
}

void Nes::tick() {
  m_cpu.tick();
}

uint8 Nes::cpu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n] = data;
  }

  case 0x2000 ... 0x3FFF: {
    //return m_ppu.cpu_write(n & 0x0007, data);
  }

  case 0x8000 ... 0xFFFF: {
    //return m_rom.cpu_write(n, data);
  }

  default: return {};
  }
}

uint8 Nes::cpu_read(uint16 n) {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x07FF];
  }

  case 0x2000 ... 0x3FFF: {
    // return m_ppu.cpu_read(n & 0x0007);
  }

  case 0x8000 ... 0xFFFF: {
    // return m_rom.cpu_read(n);
  }

  default: return {};
  }
}

uint8 Nes::ppu_write(uint16 n, uint8 data) {
  return {};
}

uint8 Nes::ppu_read(uint16 n) {
  return {};
}

uint8 Nes::apu_write(uint16 n, uint8 data) {
  return {};
}

uint8 Nes::apu_read(uint16 n) {
  return {};
}

}  // namespace nemu
