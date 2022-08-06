#include "nes.hpp"
#include "exception.hpp"

namespace nemu {

Nes::Nes(Rom &rom) : m_ppu {this}, m_gamepads {{this}, {this}}, m_rom {rom} {}

void Nes::init() {
  m_cpu.init();
  m_ppu.init();
  m_gamepads[0].init();
  m_gamepads[1].init();
}

void Nes::tick() {
  m_ppu.tick();
  m_ppu.tick();
  m_ppu.tick();
  m_cpu.tick();
  m_gamepads[0].tick();
  m_gamepads[1].tick();
}

uint8 Nes::cpu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n] = data;
  }

  case 0x2000 ... 0x3FFF: {
    return m_ppu.cpu_write(n, data);
  }

  case 0x4000 ... 0x4013:
  case 0x4015: {
    return {};  // TODO: APU write from CPU
  }

  case 0x4014: {
    return {};  // TODO: OAM write from CPU
  }

  case 0x4016: {
    return m_gamepads[0].cpu_write(n, data);
  }

  case 0x4017: {
    return m_gamepads[1].cpu_write(n, data);
  }

  case 0x4020 ... 0xFFFF: {
    return m_rom.cpu_write(n, data);
  }

  default: throw Exception {"Out of bound CPU write: 0x{:04X}", n};
  }
}

uint8 Nes::cpu_read(uint16 n) {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x07FF];
  }

  case 0x2000 ... 0x3FFF: {
    return m_ppu.cpu_read(n);
  }

  case 0x4000 ... 0x4015: {
    return {};  // TODO: APU read from CPU
  }

  case 0x4016: {
    return m_gamepads[0].cpu_read(n);
  }

  case 0x4017: {
    return m_gamepads[1].cpu_read(n);
  }

  case 0x4020 ... 0xFFFF: {
    return m_rom.cpu_read(n);
  }

  default: return {};  // throw Exception {"Out of bound CPU read: 0x{:04X}", n};
  }
}

uint8 Nes::cpu_peek(uint16 n) const {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x07FF];
  }

  case 0x2000 ... 0x3FFF: {
    return m_ppu.cpu_peek(n);
  }

    // case 0x4000 ... 0x4015: {
    //   return {};  // TODO: APU peek from CPU
    // }

  case 0x4016: {
    return m_gamepads[0].bits();
  }

  case 0x4017: {
    return m_gamepads[1].bits();
  }

  case 0x4020 ... 0xFFFF: {
    return m_rom.cpu_peek(n);
  }

  default: return {};
  }
}

uint8 Nes::ppu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x0000 ... 0x1FFF: return m_rom.ppu_write(n, data);
  }

  throw Exception {"Out of bound PPU write: 0x{:04X}", n};
}

uint8 Nes::ppu_peek(uint16 n) const {
  switch (n) {
  case 0x0000 ... 0x1FFF: return m_rom.ppu_peek(n);
  }

  return {};
}

uint8 Nes::ppu_read(uint16 n) {
  switch (n) {
  case 0x0000 ... 0x1FFF: return m_rom.ppu_read(n);
  }

  throw Exception {"Out of bound PPU read: 0x{:04X}", n};
}

}  // namespace nemu
