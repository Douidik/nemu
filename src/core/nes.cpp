#include "nes.hpp"
#include "exception.hpp"
#include "mapper/mapper.hpp"

namespace nemu {

Nes::Nes(Rom &rom) : m_ppu {this}, m_gamepads {{this}, {this}}, m_mapper {Mapper::create(rom)} {}

void Nes::init() {
  m_mapper->init();
  m_cpu.init();
  m_ppu.init();
  m_gamepads[0].init();
  m_gamepads[1].init();
}

void Nes::tick() {
  m_ppu.tick();
  m_ppu.tick();
  m_ppu.tick();

  if (m_dma) {
    if (m_dma->w ^= 1) {
      m_dma->buffer = cpu_read(m_dma->page << 8 | m_dma->address);
    } else {
      m_ppu.dma_write(m_dma->address, m_dma->buffer);

      if (m_dma->address != 0xFF) {
        m_dma->address++;
      } else {
        m_dma = std::nullopt;
      }
    }
  } else {
    m_cpu.tick();
  }

  m_gamepads[0].tick();
  m_gamepads[1].tick();
}

uint8 Nes::cpu_write(uint16 n, uint8 data) {
  if (uint8 *mapper_write = m_mapper->cpu_write(n, data)) {
    return *mapper_write;
  }

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x07FF] = data;
  }

  case 0x2000 ... 0x3FFF: {
    return m_ppu.cpu_write(n, data);
  }

  case 0x4000 ... 0x4013:
  case 0x4015: {
    return {};  // TODO: APU write from CPU
  }

  case 0x4014: {
    return (m_dma = ppu::Dma {data, 0x00, {}, 0})->page;
  }

  case 0x4016: {
    return m_gamepads[0].cpu_write(n, data);
  }

  case 0x4017: {
    return m_gamepads[1].cpu_write(n, data);
  }

    // default: throw Exception {"Out of bound CPU write: 0x{:04X}", n};
  }

  return {};
}

uint8 Nes::cpu_peek(uint16 n) const {
  if (const uint8 *mapper_peek = m_mapper->cpu_peek(n)) {
    return *mapper_peek;
  }

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x07FF];
  }

  case 0x2000 ... 0x3FFF: {
    return m_ppu.cpu_peek(n);
  }

  case 0x4000 ... 0x4015: {
    return {};  // TODO: APU peek from CPU
  }

  case 0x4016: {
    return m_gamepads[0].bits();
  }

  case 0x4017: {
    return m_gamepads[1].bits();
  }

  default: return {};
  }
}

uint8 Nes::cpu_read(uint16 n) {
  if (uint8 *mapper_read = m_mapper->cpu_read(n)) {
    return *mapper_read;
  }

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_ram[n & 0x7FF];
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

    // default: throw Exception {"Out of bound CPU read: 0x{:04X}", n};
  }

  return {};
}

uint8 Nes::ppu_write(uint16 n, uint8 data) {
  uint8 *mapper_write = m_mapper->ppu_write(n, data);

  if (!mapper_write) {
    throw Exception {"Out of bounds PPU write: 0x{:04X}", n};
  }

  return *mapper_write;
}

uint8 Nes::ppu_peek(uint16 n) const {
  const uint8 *mapper_peek = m_mapper->ppu_peek(n);

  if (!mapper_peek) {
    return {};
  }

  return *mapper_peek;
}

uint8 Nes::ppu_read(uint16 n) {
  uint8 *mapper_read = m_mapper->ppu_read(n);

  if (!mapper_read) {
    throw Exception {"Out of bounds PPU read: 0x{:04X}", n};
  }

  return *mapper_read;
}

}  // namespace nemu
