#include "ppu.hpp"
#include "bus.hpp"
#include "exception.hpp"
#include "int.hpp"
#include "nes.hpp"
#include "ppu/registers.hpp"
#include "rom.hpp"

namespace nemu {

Ppu::Ppu(Nes *nes) : Hardware {nes} {}

void Ppu::init() {
  m_data_buffer = {}, m_vram = {}, m_latch = {}, m_palette = {};
  m_scanline = 0, m_ticks = 0, m_frame_count = 0;

  m_regs = PpuRegisters {
    .oam = {{0x00}, 0x00, 0x00},
    .vram_address = {},
    .temp_address = {},
    .fine_x = 0x00,
    .control = 0x00,
    .mask = 0x00,
    .status = 0x00,
  };
}

void Ppu::tick() {
  // NOTE: We don't emulate the PPU behaviour when it comes to actual rendering

  switch (m_scanline) {
  case 241: {  // V-blank
    if (m_ticks == 1) {
      m_bus.cpu().nmi();
      m_regs.status |= PPU_VBLANK;
    }
  } break;

  case 261: {  // Pre-render
    switch (m_ticks) {
    case 1: m_regs.status &= ~PPU_VBLANK; break;
    case 340: m_scanline = 0, m_frame_count++; break;
    }
  } break;
  }

  if (m_ticks > 341) {
    m_ticks = 0, m_scanline++;
  } else {
    m_ticks++;
  }
}

Canvas Ppu::render_canvas() {
  Canvas canvas_background {}, canvas_foreground {};

  for (uint16 x = 0; x < CANVAS_W; x++) {
    for (uint16 y = 0; y < CANVAS_H; y++) {
      canvas_background.buffer[x][y] = render_background(x, y);
      canvas_foreground.buffer[x][y] = render_foreground(x, y);
    }
  }

  return canvas_background;
}

uint8 Ppu::cpu_read(uint16 n) {
  switch (n) {
  case 0x2002: {
    uint8 output = m_regs.status;

    m_latch = false;
    m_regs.status &= ~PPU_VBLANK;

    return output;
  }

  case 0x2004: {
    return m_regs.oam.data[m_regs.oam.address];
  }

  case 0x2007: {
    return ppu_read(increment_vram_address());
  }
  }

  throw Exception {"Out of bound PPU read from CPU: 0x{:04X}", n};
}

uint8 Ppu::cpu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x2000: {
    m_regs.temp_address.nametable_x = data & 0x01;
    m_regs.temp_address.nametable_y = data & 0x02;

    return m_regs.control |= (data & PPU_NAMETABLE_ADDRESS);
  }

  case 0x2001: {
    return m_regs.mask = data;
  }

  case 0x2003: {
    return m_regs.oam.address = data;
  }

  case 0x2004: {
    return m_regs.oam.data[m_regs.oam.address] = data;
  }

  case 0x2005: {
    if (!(m_latch = !m_latch)) {
      m_regs.fine_x = data & 0x07;
      m_regs.temp_address.coarse_x = data >> 3;
    } else {
      m_regs.temp_address.fine_y = data & 0x07;
      m_regs.temp_address.coarse_y = data >> 3;
    }
  }

  case 0x2006: {
    if (!(m_latch = !m_latch)) {
      return m_regs.temp_address.bits |= (data & 0x3F) << 8;
    } else {
      return m_regs.vram_address.bits = (m_regs.temp_address.bits |= data);
    }
  }

  case 0x2007: {
    return ppu_write(increment_vram_address(), data);
  }
  }

  throw Exception {"Out of bound PPU write from CPU: 0x{:04X}", n};
}

uint8 Ppu::ppu_read(uint16 n) {
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_bus.ppu_read(n);
  }

  case 0x2000 ... 0x3EFF: {
    return m_vram[parse_vram_address(n)];
  }

  case 0x3F00 ... 0x3FFF: {
    switch (n) {
    case 0x3F10:
    case 0x3F14:
    case 0x3F18:
    case 0x3F1C: {
      return ppu_read(n - 0x10);
    }
    }

    return m_palette[n - 0x3F00];
  }
  }

  throw Exception {"Out of bound PPU read (with address register) from CPU: 0x{:04X}", n};
}

uint8 Ppu::ppu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x2000 ... 0x3EFF: {
    return m_vram[parse_vram_address(n)] = data;
  }

  case 0x3F00 ... 0x3FFF: {
    switch (n) {
    case 0x3F10:
    case 0x3F14:
    case 0x3F18:
    case 0x3F1C: {
      return ppu_write(n - 0x10, data);
    }
    }

    return m_palette[n - 0x3F00] = data;
  }
  }

  throw Exception {"Out of bound PPU write (with address register) from CPU: 0x{:04X}", n};
}

auto Ppu::fetch_pattern(uint8 n) {
  std::array<uint8, 8> pattern_a {}, pattern_b {};

  for (uint8 i = 0; i < 8; i++) {
    pattern_a[i] = ppu_read(n * 16 + i);
    pattern_b[i] = ppu_read(n * 16 + i + 8);
  }

  return std::make_tuple(pattern_a, pattern_b);
}

uint8 Ppu::fetch_nametable(uint8 n) {
  uint16 offset = (m_regs.control & PPU_NAMETABLE_ADDRESS) * 0x0400;
  uint16 base_nametable = 0x2000 + offset;
  uint16 address = base_nametable | n;

  return ppu_read(address);
}

uint16 Ppu::parse_vram_address(uint16 address) {
  uint16 mapped = address & 0x0FFF;
  uint8 nametable = mapped / 0x0400;

  switch (m_bus.rom().mirror()) {
  case Mirror::HORIZONTAL: {
    switch (nametable) {
    case 0: return mapped - 0x0000;
    case 1: return mapped - 0x0400;
    case 2: return mapped - 0x0400;
    case 3: return mapped - 0x0800;
    }
  }
  case Mirror::VERTICAL: {
    switch (nametable) {
    case 0: return mapped - 0x0000;
    case 1: return mapped - 0x0000;
    case 2: return mapped - 0x0800;
    case 3: return mapped - 0x0800;
    }
  }
  }

  return {};
}

uint16 Ppu::increment_vram_address() {
  uint16 address = m_regs.vram_address.bits;

  if (m_regs.control & PPU_VRAM_INCREMENT) {
    m_regs.vram_address.bits += 32;
  } else {
    m_regs.vram_address.bits += 1;
  }

  return address;
}

uint8 Ppu::render_background(uint16 x, uint16 y) {
  uint16 index = (x / 8) + (y / 8) * 32;
  uint8 tile = fetch_nametable(index);
  
  auto [pattern_a, pattern_b] = fetch_pattern(tile);

  uint8 a = pattern_a[y % 8] & (1 << (x % 8)) ? 0b0'1 : 0b0'0;
  uint8 b = pattern_b[y % 8] & (1 << (x % 8)) ? 0b1'0 : 0b0'0;

  return a | b;
}

uint8 Ppu::render_foreground(uint16 x, uint16 y) {
  return {};
}

}  // namespace nemu

// void Ppu::render() {
//   for (uint16 n = 0; n < 960; n++) {
//     uint8 tile = fetch_nametable(n);
//     auto [main_pattern, second_pattern] = fetch_pattern(tile);

//     for (uint8 i = 0; i < 8; i++) {
//       for (uint8 j = 0; j < 8; j++) {
//         uint8 a = main_pattern[i] & (1 << j);
//         uint8 b = second_pattern[i] & (1 << j);

//         auto [x, y] = std::div(n, 32);
//         // canvas.buffer[x * 8 + j][y * 8 + i] = a | (b << 1);
//       }
//     }
//   }

//   uint8 tile_x = (m_ticks / 8) % 64;
//   uint8 tile_y = (m_ticks / 8) % 60;

//   uint8 tile = fetch_nametable((tile_x + tile_y))
// }
