#include "ppu.hpp"
#include "exception.hpp"
#include "int.hpp"
#include "mapper/mapper.hpp"
#include "mapper/mapper_mmc1.hpp"
#include "misc.hpp"
#include "nes.hpp"
#include "sprite.hpp"
#include <tuple>

namespace nemu {

using namespace ppu;

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
  ppu_event("render_first_tick", 0, 0, [this] {
    if ((m_regs.mask.bgr_show) && (m_framecount & 0b1)) {
      m_ticks = 1;  // Skipped on Odd + Background
    }
  });

  ppu_event("render_finished", 0, 240, [this] {
    render_background(m_canvas);
    render_sprites(m_canvas);
  });

  ppu_event("set_vblank", 1, 241, [this] {
    m_regs.status.vblank = 1;

    if (m_regs.control.nmi) {
      m_bus.cpu().nmi();
    }
  });

  ppu_event("end_vblank", 1, -1, [this] {
    m_regs.status.vblank = 0;
    m_regs.status.spr_zero_hit = 0;
  });

  // end_tick
  m_ticks++;

  ppu_event("end_frame", 341, 260, [this] {
    m_regs.status.spr_zero_hit = 0, m_scanline = -1, m_ticks = 1, m_framecount++;
  });

  ppu_event("end_scaline", 341, std::nullopt, [this] {
    Sprite spr_zero = {{m_oam[3], m_oam[0]}, m_oam[1], m_oam[2]};

    m_regs.status.spr_zero_hit = {
      m_regs.mask.spr_show && spr_zero.position[0] <= m_ticks && spr_zero.position[1] == m_scanline,
    };

    m_scanline++, m_ticks = 0;
  });
}

uint8 Ppu::dma_write(uint8 n, uint8 data) {
  return m_oam[n] = data;
}

uint8 Ppu::cpu_write(uint16 n, uint8 data) {
  switch (n & 0x2007) {
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

  return {};
}

uint8 Ppu::cpu_peek(uint16 n) const {
  switch (n & 0x2007) {
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
  switch (n & 0x2007) {
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

  return {};
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

  return {};
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

  switch (m_bus.mapper()->mirror()) {
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

    // NOTE: assume that both types or mirroring acts similarly 8==D

  case Mirror::ONE_SCREEN_LO:
  case Mirror::ONE_SCREEN_UP: {
    return mapped & 0x03FF;
  }

  default: break;
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

Canvas &Ppu::render_background(Canvas &canvas) const {
  if (!m_regs.mask.bgr_show) {
    return canvas;
  }

  uint8 bank = m_regs.control.bgr_bank;
  auto pattern = m_bus.mapper()->pattern(bank);

  for (uint16 i = 0; i < Canvas::W; i++) {
    for (uint16 j = 0; j < Canvas::H; j++) {
      // Get the scroll wrapped around the two nametables
      uint16 x = (m_regs.scroll.x + i + (m_regs.control.nt_x * Canvas::W)) % (Canvas::W * 2);
      uint16 y = (m_regs.scroll.y + j + (m_regs.control.nt_y * Canvas::H)) % (Canvas::H * 2);

      // Inner nametable tile coordinates
      uint8 r = (x % Canvas::W) / 8;
      uint8 c = (y % Canvas::H) / 8;

      // Select from which nametable we are rendering depending on the current scroll
      uint8 n = x >= Canvas::W || y >= Canvas::H;

      uint16 nt_index = r + c * (Canvas::W / 8);
      uint16 nt_value = m_vram[(n * 0x400) + nt_index];

      auto pattern_a = pattern.subspan(nt_value * 16 + 0, 8);
      auto pattern_b = pattern.subspan(nt_value * 16 + 8, 8);

      uint8 a = pattern_a[y % 8] & (1 << (7 - x % 8)) ? 0b0'1 : 0b0'0;
      uint8 b = pattern_b[y % 8] & (1 << (7 - x % 8)) ? 0b1'0 : 0b0'0;

      uint8 half_a = (uint8(r / 2) & 0b1) ? 0b0'1 : 0b0'0;
      uint8 half_b = (uint8(c / 2) & 0b1) ? 0b1'0 : 0b0'0;

      uint8 quadrant = (half_a | half_b);
      uint8 ab_index = (r / 4) + (c / 4) * (Canvas::W / 8 / 4);
      uint8 ab_value = (m_vram[(n * 0x400 + 0x3C0) + ab_index] >> (2 * quadrant)) & 0b11;

      if (a | b) {
        canvas.buffer[i][j] = m_colors[(ab_value << 2) | (a | b)];
      } else {
        canvas.buffer[i][j] = m_colors[0x00];  // Draw background color
      }
    }
  }

  return canvas;
}

Canvas &Ppu::render_sprites(Canvas &canvas) const {
  if (!m_regs.mask.spr_show) {
    return canvas;
  }

  auto render_sprite = [&](Sprite sprite, uint8 bank) {
    auto pattern = m_bus.mapper()->pattern(bank);

    auto pattern_a = pattern.subspan(sprite.index * 16 + 0, 8);
    auto pattern_b = pattern.subspan(sprite.index * 16 + 8, 8);

    for (uint8 c = 0; c < 8; c++) {
      for (uint8 r = 0; r < 8; r++) {
        uint16 x = sprite.position[0] + (sprite.ab.flip & 0b0'1 ? r : 7 - r);
        uint16 y = sprite.position[1] + (sprite.ab.flip & 0b1'0 ? 7 - c : c);

        if (y < 2 || x > Canvas::W || y > Canvas::H) {
          return;  // Sprite is not on the screen anymore
        }

        uint8 a = pattern_a[c] & (1 << r) ? 0b0'1 : 0b0'0;
        uint8 b = pattern_b[c] & (1 << r) ? 0b1'0 : 0b0'0;

        // The sprite either need to be opaque or have the priority to be displayed
        if ((a | b) && (canvas.buffer[x][y] == m_colors[0x00] || sprite.ab.priority < 1)) {
          canvas.buffer[x][y] = m_colors[0x10 + (sprite.ab.color << 2) | (a | b)];
        }
      }
    }
  };

  if (m_regs.control.spr_size < 1) {
    for (uint8 n = 64; n > 0; n--) {
      render_sprite(Sprite::from_span({&m_oam[(n - 1) * 4], 4}), m_regs.control.spr_bank);
    }
  } else {
    for (uint8 n = 64; n > 0; n--) {
      Sprite sprite_a = Sprite::from_span({&m_oam[(n - 1) * 4], 4});
      Sprite sprite_b = Sprite::from_span({&m_oam[(n - 1) * 4], 4});

      // Should be the same
      uint8 bank = (sprite_a.index & 0b1) | (sprite_b.index & 0b1);

      // The index is stored in the upper 7 bits, sprite b takes the next index
      sprite_a.index = (sprite_a.index >> 0) + 0;
      sprite_b.index = (sprite_b.index >> 0) + 1;

      // Vertical flip changes the position of sprite_b if it occupies an odd y position
      if (sprite_b.ab.flip & 0b1'0) {
        sprite_b.position[1] += 16;
      } else {
        sprite_b.position[1] += 8;
      }

      render_sprite(sprite_a, bank);
      render_sprite(sprite_b, bank);
    }
  }

  return canvas;
}

}  // namespace nemu
