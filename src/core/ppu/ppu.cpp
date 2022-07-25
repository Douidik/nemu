#include "ppu.hpp"
#include "exception.hpp"
#include "nes.hpp"
#include <tuple>

namespace nemu {

template<typename T>
class Range {
public:
  Range(auto a, auto b) : bounds {a, b} {}
  Range(auto n) : bounds {n, n} {}
  Range() : bounds {std::nullopt, std::nullopt} {}

  bool includes(auto n) const {
    return (!bounds[0] || n >= *bounds[0]) && (!bounds[1] || *bounds[1] >= n);
  }

private:
  std::optional<T> bounds[2];
};

Ppu::Ppu(Nes *nes) : Hardware {nes} {}

void Ppu::init() {
  m_vram = {}, m_colors = {};
  m_scanline = 0, m_ticks = 0, m_framecount = 0;

  m_regs = {
    .address = {},
    .scroll = {},
    .latch = {},
    .control = {},
    .mask = {},
    .status = {},
  };
}

void Ppu::tick() {
  auto ppu_event = [this](
                     std::string_view timing,
                     Range<uint32> tick,
                     Range<uint32> scanline,
                     auto event) -> bool {
    bool included = tick.includes(m_ticks) && scanline.includes(m_scanline);

    if (included) {
      m_timing = timing;
      event();
    }

    return included;
  };

  ppu_event("begin_vblank", 1, 241, [this] {
    m_regs.status.vblank = 1;

    if (m_regs.control.nmi) {
      m_bus.cpu().nmi();
    }
  });

  ppu_event("end_vblank", 1, 261, [this] {
    m_regs.status.vblank = 0;
  });

  ppu_event("begin_frame", 340, 261, [this] {
    m_scanline = 0, m_framecount++;
  });

  // end_tick
  m_ticks++;

  ppu_event("end_scaline", 341, std::nullopt, [this] {
    m_scanline++, m_ticks = 0;
  });

  // if (!(m_framecount & 2) && on_event(0, 0, "skipped_on_odd")) {
  //   m_ticks++;
  // }
}

std::optional<uint8> Ppu::cpu_peek(uint16 n) const {
  switch (n) {
  case 0x2002: {
    return m_regs.status.bits;
  }

  case 0x2004: {
    return {};  // TODO
  }

  case 0x2007: {
    return ppu_peek();
  }
  }

  return std::nullopt;
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
    return {};
  }

  case 0x2004: {
    return {};
  }

  case 0x2005: {
    if (m_regs.latch.use()) {
      return m_regs.scroll.x = data;
    } else {
      return m_regs.scroll.y = data;
    }
  }

  case 0x2006: {
    if (m_regs.latch.use()) {
      return m_regs.address |= (data << 8);
    } else {
      return m_regs.address |= (data & 0xFF);
    }
  }

  case 0x2007: {
    return ppu_write(data);
  }
  }

  throw Exception {"Out of bound PPU write from CPU: 0x{:04X}", n};
}

uint8 Ppu::cpu_read(uint16 n) {
  switch (n) {
  case 0x2002: {
    uint8 bits = m_regs.status.bits;

    m_regs.latch.reset();
    m_regs.status.vblank = 0;

    return bits;
  }

  case 0x2004: {
    return {};
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

std::optional<uint8> Ppu::ppu_peek() const {
  uint16 n = m_regs.address;

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_bus.ppu_peek(n);
  }

  case 0x2000 ... 0x3EFF: {
    return m_vram[vram_address(n)];
  }

  case 0x3F00 ... 0x3FFF: {
    return m_colors[color_address(n)];
  }
  }

  return std::nullopt;
}

uint8 Ppu::ppu_write(uint8 data) {
  uint16 n = ppu_address();

  switch (n) {
  case 0x2000 ... 0x3EFF: {
    return m_vram[vram_address(n)] = data;
  }

  case 0x3F00 ... 0x3FFF: {
    return m_colors[color_address(n)] = data;
  }
  }

  throw Exception {"Out of bound PPU write (with the address register) from CPU: 0x{:04X}", n};
}

uint8 Ppu::ppu_read() {
  uint16 n = ppu_address();

  switch (n) {
  case 0x0000 ... 0x1FFF: {
    return m_bus.ppu_read(n);
  }

  case 0x2000 ... 0x3EFF: {
    return m_vram[vram_address(n)];
  }

  case 0x3F00 ... 0x3FFF: {
    return m_colors[color_address(n)];
  }
  }

  throw Exception {"Out of bound PPU read (with the address register) from CPU: 0x{:04X}", n};
}

uint16 Ppu::ppu_address() {
  uint16 output = m_regs.address;

  if (m_regs.control.vram_increment) {
    m_regs.address = (m_regs.address + 0x20);  // & 0x3FFF;
  } else {
    m_regs.address = (m_regs.address + 0x01);  // & 0x3FFF;
  }

  return output;
}

uint16 Ppu::vram_address(uint16 n) const {
  uint16 mapped = n - 0x2000;
  uint8 nametable = mapped / 0x0400;

  switch (m_bus.rom().mirror()) {
  case Mirror::HORIZONTAL: {
    switch (nametable) {
    case 0: return mapped - 0x000;
    case 1: return mapped - 0x400;
    case 2: return mapped - 0x400;
    case 3: return mapped - 0x800;
    }
  }

  case Mirror::VERTICAL: {
    switch (nametable) {
    case 0: return mapped - 0x000;
    case 1: return mapped - 0x000;
    case 2: return mapped - 0x800;
    case 3: return mapped - 0x800;
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

  auto pattern = m_bus.rom().pattern(0);

  for (uint16 x = 0; x < Canvas::W; x++) {
    for (uint16 y = 0; y < Canvas::H; y++) {
      uint16 nt_index = 0x2000 | ((x / 8) + (y / 8) * Canvas::W);
      uint16 nt_value = m_vram[vram_address(nt_index)];

      auto pattern_a = pattern.subspan(nt_value * 16 + 0, 8);
      auto pattern_b = pattern.subspan(nt_value * 16 + 8, 8);

      uint8 a = pattern_a[y % 8] & (1 << (x % 8)) ? 0b0'1 : 0b0'0;
      uint8 b = pattern_b[y % 8] & (1 << (x % 8)) ? 0b1'0 : 0b0'0;

      canvas.buffer[x][y] = a | b;
    }
  }

  return canvas;
}

Canvas Ppu::render_foreground() {
  return {};
}

}  // namespace nemu
