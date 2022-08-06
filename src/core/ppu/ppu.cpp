#include "ppu.hpp"
#include "exception.hpp"
#include "int.hpp"
#include "misc.hpp"
#include "nes.hpp"
#include <tuple>

namespace nemu {

Ppu::Ppu(Nes *nes) : Hardware {nes} {}

void Ppu::init() {
  m_vram = {}, m_colors = {};
  m_scanline = 0, m_ticks = 0, m_framecount = 0;

  m_regs = {
    .w = 0,
    .vram_address = {},
    .scroll = {},
    .control = {},
    .mask = {},
    .status = {},
  };
}

void Ppu::tick() {
  auto ppu_event = [this](
                     std::string_view timing,
                     std::optional<int32> ticks,
                     std::optional<int32> scanline,
                     auto event) {
    if ((!ticks || m_ticks == ticks) && (!scanline || m_scanline == scanline)) {
      m_timing = timing, event();
    }
  };

  ppu_event("render_first_tick", 0, 0, [this] {
    if ((m_regs.mask.bgr_show) && (m_ticks & 0b1)) {
      m_ticks = 1;  // Skipped on Odd + Background
    }
  });

  ppu_event("set_vblank", 1, 241, [this] {
    m_regs.status.vblank = 1;

    if (m_regs.control.nmi) {
      m_bus.cpu().nmi();
    }
  });

  ppu_event("end_vblank", 1, -1, [this] {
    // TODO: sprite zero hit
    m_regs.status.vblank = 0;
  });

  // end_tick
  m_ticks++;

  ppu_event("end_frame", 341, 260, [this] {
    m_scanline = -1, m_ticks = 1, m_framecount++;
  });

  ppu_event("end_scaline", 341, std::nullopt, [this] {
    m_scanline++, m_ticks = 0;
  });
}

uint8 Ppu::cpu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x2000: {
    return m_regs.control.bits = data;
  }

  case 0x2001: {
    return m_regs.mask.bits = data;
  }

  case 0x2003: {
    return m_regs.oam_address = data;
  }

  case 0x2004: {
    return m_oam[m_regs.oam_address] = data;
  }

  case 0x2005: {
    if (m_regs.w ^= 1) {
      return m_regs.scroll.x = data;
    } else {
      return m_regs.scroll.y = data;
    }
  }

  case 0x2006: {
    if (m_regs.w ^= 1) {
      return m_regs.vram_address = m_regs.vram_address & 0x00FF | (data << 0x8);
    } else {
      return m_regs.vram_address = m_regs.vram_address & 0xFF00 | (data & 0xFF);
    }
  }

  case 0x2007: {
    return ppu_write(data);
  }
  }

  throw Exception {"Out of bound PPU write from CPU: 0x{:04X}", n};
}

uint8 Ppu::cpu_peek(uint16 n) const {
  switch (n) {
  case 0x2000: {
    return m_regs.control.bits;
  }

  case 0x2001: {
    return m_regs.mask.bits;
  }

  case 0x2002: {
    return m_regs.status.bits;
  }

  case 0x2003: {
    return m_regs.oam_address;
  }

  case 0x2004: {
    return m_oam[m_regs.oam_address];
  }

  case 0x2005: {
    if (m_regs.w ^ 1) {
      return m_regs.scroll.x;
    } else {
      return m_regs.scroll.y;
    }
  }

  case 0x2006: {
    if (m_regs.w ^ 1) {
      return m_regs.vram_address >> 0x8;
    } else {
      return m_regs.vram_address & 0xFF;
    }
  }

  case 0x2007: {
    return ppu_peek();
  }
  }

  return {};
}

uint8 Ppu::cpu_read(uint16 n) {
  switch (n) {
  case 0x2002: {
    uint8 bits = m_regs.status.bits;

    m_regs.w = 0;
    m_regs.status.vblank = 0;

    return bits;
  }

  case 0x2004: {
    return m_oam[m_regs.oam_address];
  }

  case 0x2007: {
    return ppu_read();
  }
  }

  throw Exception {"Out of bound PPU read from CPU: 0x{:04X}", n};
}

Canvas Ppu::render_canvas() {
  return render_background();
}

uint8 Ppu::ppu_write(uint8 data) {
  uint16 n = ppu_address();

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_bus.ppu_write(n, data);
  }

  case 0x2000 ... 0x3EFF: {
    return m_vram[vram_address(n)] = data;
  }

  case 0x3F00 ... 0x3FFF: {
    return m_colors[color_address(n)] = data;
  }
  }

  throw Exception {"Out of bound PPU write (with address register) from CPU: 0x{:04X}", n};
}

uint8 Ppu::ppu_peek() const {
  uint16 n = m_regs.vram_address & 0x3FFF;

  switch (n) {
  case 0x0000 ... 0x3EFF: {
    return m_regs.buffer;
  }

  case 0x3F00 ... 0x3FFF: {
    return m_colors[color_address(n)];
  }

  default: {
    return {};
  }
  }
}

uint8 Ppu::ppu_read() {
  uint8 output = m_regs.buffer;
  uint16 n = ppu_address();

  // PPU Reads are done with one cycle delay excepted for the palette ram which we read from directly
  switch (n) {
  case 0x0000 ... 0x1FFF: {
    m_regs.buffer = m_bus.ppu_read(n);
  } break;

  case 0x2000 ... 0x3EFF: {
    m_regs.buffer = m_vram[vram_address(n)];
  } break;

  case 0x3F00 ... 0x3FFF: {
    output = m_regs.buffer = m_colors[color_address(n)];
  } break;

  default: {
    throw Exception {"Out of bound PPU read (with address register) from CPU: 0x{:04X}", n};
  } break;
  }

  return output;
}

uint16 Ppu::ppu_address() {
  // Map the address to PPU space [$0000-$3FFF]
  uint16 output = m_regs.vram_address &= 0x3FFF;

  if (m_regs.control.vram_increment) {
    m_regs.vram_address += 32;
  } else {
    m_regs.vram_address += 1;
  }

  return output;
}

uint16 Ppu::vram_address(uint16 n) const {
  // Map the address to nametable space [$2000-$3EFF] -> [$000-$FFF]
  uint16 mapped = n & 0x0FFF;

  // Which nametable is selected [0-3]
  uint8 nametable = mapped / 0x400;

  switch (m_bus.rom().mirror()) {
  case Mirror::HORIZONTAL: {
    switch (nametable) {
    case 0: return mapped - 0x000;  // A
    case 1: return mapped - 0x400;  // A'
    case 2: return mapped - 0x400;  // B
    case 3: return mapped - 0x800;  // B'
    }
  }

  case Mirror::VERTICAL: {
    switch (nametable) {
    case 0: return mapped - 0x000;  // A
    case 1: return mapped - 0x000;  // B
    case 2: return mapped - 0x800;  // A'
    case 3: return mapped - 0x800;  // B'
    }
  }
  }

  return {};
}

uint16 Ppu::color_address(uint16 n) const {
  uint8 mapped = n & 0x001F;

  switch (mapped) {
  case 0x10:
  case 0x14:
  case 0x18:
  case 0x1C: {
    return mapped - 0x10;
  }
  }

  return mapped;
}

Canvas Ppu::render_background() {
  Canvas canvas {};

  uint8 bank = m_regs.control.bgr_bank;
  const auto pattern = m_bus.rom().pattern(bank);
  const auto &scroll = m_regs.scroll;

  for (uint16 i = 0; i < Canvas::W; i++) {
    for (uint16 j = 0; j < Canvas::H; j++) {
      uint8 x = i / 8;
      uint8 y = j / 8;
      uint8 r = i % 8;
      uint8 c = j % 8;

      // uint16 nt_index = (scroll.x + x) + (scroll.y + y) * Canvas::W / 8;

      // // Access the second nametable
      // if (nt_index > 0x38F) {
      //   nt_index = (0x400 + (nt_index - 0x3C0)) % 0x800;
      // }

      // uint16 nt_value = m_vram[nt_index];

      uint16 nt_value = m_vram[x + y * Canvas::W / 8];

      auto pattern_a = pattern.subspan(nt_value * 16 + 0, 8);
      auto pattern_b = pattern.subspan(nt_value * 16 + 8, 8);

      uint8 a = pattern_a[c] & (1 << (7 - r)) ? 0b0'1 : 0b0'0;
      uint8 b = pattern_b[c] & (1 << (7 - r)) ? 0b1'0 : 0b0'0;

      uint8 half_a = (uint8(x / 2) & 0b1) ? 0b0'1 : 0b0'0;
      uint8 half_b = (uint8(y / 2) & 0b1) ? 0b1'0 : 0b0'0;

      uint8 quadrant = (half_a | half_b);
      uint8 ab_index = (x / 4) + (y / 4) * (Canvas::W / 8 / 4);
      uint8 ab_value = (m_vram[0x3C0 + ab_index] >> (2 * quadrant)) & 0b11;
      
      canvas.buffer[i][j] = m_colors[(ab_value << 2) | (a | b)];
    }
  }

  return canvas;
}

Canvas Ppu::render_foreground() {
  return {};
}

}  // namespace nemu
