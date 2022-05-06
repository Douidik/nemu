#include "ppu.hpp"
#include "bus.hpp"
#include "nes.hpp"
#include "rom.hpp"

namespace nemu {

Ppu::Ppu(Nes *nes) : Hardware {nes} {}

void Ppu::init() {}

void Ppu::tick() {
  switch (m_scanline) {
  case 261: return pre_render();
  case 0 ... 239: return render();
  case 240 ... 260: return post_render();
  }
}

void Ppu::pre_render() {}

void Ppu::render() {}

void Ppu::post_render() {}

uint8 Ppu::cpu_read(uint16 n) {
  // Register access
  switch (n) {
  case 0x2002: {
    return m_regs.status |= PPU_VBLANK;
  }

  case 0x2004: {
    return m_regs.oam.data[m_regs.oam.address];
  }

  case 0x2007: {
    return m_bus.ppu_read(increment_vram_address());
  }
  }

  // PPU access
  switch (n) {
    // VRAM access
  case 0x2000 ... 0x3EFF: {
    return m_vram[parse_vram_address(n)];
  }

    // Palette access
  case 0x3F00 ... 0x3FFF: {
    switch (n) {
    case 0x3F10:
    case 0x3F14:
    case 0x3F18:
    case 0x3F1C: {
      return cpu_read(n - 0x10);
    }
    }

    return m_palette[n - 0x3F00];
  }
  }

  return {};
}

uint8 Ppu::cpu_write(uint16 n, uint8 data) {
  // Register access
  switch (n) {
  case 0x2000: {
    m_regs.temp_address.nametable_x = data & 0x01;
    m_regs.temp_address.nametable_y = data & 0x02;

    return m_regs.control |= data & PPU_NAMETABLE_ADDRESS;
  }

  case 0x2001: return m_regs.mask = data;
  case 0x2003: return m_regs.oam.address = data;
  case 0x2004: return m_regs.oam.data[m_regs.oam.address] = data;

  case 0x2005: {
    if (!use_latch()) {
      m_regs.fine_x = data & 0x07;
      m_regs.temp_address.coarse_x = data >> 3;
    } else {
      m_regs.temp_address.fine_y = data & 0x07;
      m_regs.temp_address.coarse_y = data >> 3;
    }
  }

  case 0x2006: {
    if (!use_latch()) {
      return m_regs.temp_address.raw |= (data & 0x3F) << 8;
    } else {
      return m_regs.vram_address.raw = (m_regs.temp_address.raw |= data);
    }
  }

  case 0x2007: {
    return m_bus.ppu_write(increment_vram_address(), data);
  }
  }

  // PPU access
  switch (n) {
    // VRAM access
  case 0x2000 ... 0x3EFF: {
    return m_vram[parse_vram_address(n)];
  }

    // Palette access
  case 0x3F00 ... 0x3FFF: {
    switch (n) {
    case 0x3F10:
    case 0x3F14:
    case 0x3F18:
    case 0x3F1C: {
      return cpu_read(n - 0x10);
    }
    }

    return m_palette[n - 0x3F00];
  }
  }

  return {};
}

uint16 Ppu::parse_vram_address(uint16 address) const {
  // Remap the address from [0x3000 - 0x3EFF] to [0x2000 - 0x2EFF] then to vram space [0x000 - 0x2000]
  uint16 mapped = (address & 0x2FFF) - 0x2000;

  switch (m_bus.rom().mirror()) {
  case Mirror::HORIZONTAL: {
    return mapped + 0x0400;
  }
  case Mirror::VERTICAL: {
    return mapped + 0x0800;
  }
  }
}

bool Ppu::use_latch() {
  return !(m_latch = !m_latch);
}

uint16 Ppu::increment_vram_address() {
  uint16 output = m_regs.vram_address.raw;
  m_regs.vram_address.raw += m_regs.control & PPU_VRAM_INCREMENT ? 32 : 1;
  return output;
}

}  // namespace nemu
