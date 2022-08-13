#ifndef NEMU_PPU_HPP
#define NEMU_PPU_HPP

#include "hardware.hpp"
#include "misc.hpp"
#include "registers.hpp"
#include <string_view>

namespace nemu {

struct Canvas {
  enum : uint16 { W = 256, H = 240 };
  std::array<std::array<uint8, H>, W> buffer {};
};

class Ppu : public Hardware<class Nes> {
public:
  Ppu(Nes *nes);
  void init() override;
  void tick() override;

  uint8 dma_write(uint8 n, uint8 data);
  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_peek(uint16 n) const;
  uint8 cpu_read(uint16 n);

  inline const auto &regsiters() const {
    return m_regs;
  }

  inline Canvas &canvas() {
    return m_canvas;
  };

  inline int32 framecount() const {
    return m_framecount;
  }
  
private:
  template<typename F, typename R = std::invoke_result_t<F>>
  auto ppu_event(
    std::string_view timing,
    std::optional<int32> ticks,
    std::optional<int32> scanline,
    F event) -> R {
    if (ticks != std::nullopt && *ticks != m_ticks) {
      return invoke_default_value<F>();
    }

    if (scanline != std::nullopt && *scanline != m_scanline) {
      return invoke_default_value<F>();
    }

    return event();
  }

  uint8 ppu_write(uint8 data);
  uint8 ppu_peek() const;
  uint8 ppu_read();

  uint16 ppu_address();
  uint16 vram_address(uint16 n) const;
  uint16 color_address(uint16 n) const;

  Canvas &render_nametable(Canvas &canvas, uint8 n, int8 offset) const;
  Canvas &render_background(Canvas &canvas) const;
  Canvas &render_sprites(Canvas &canvas) const;

  ppu::Registers m_regs;
  Canvas m_canvas {};

  std::array<uint8, 0x100> m_oam;
  std::array<uint8, 0x800> m_vram;
  std::array<uint8, 0x020> m_colors;

  int32 m_scanline, m_ticks, m_framecount;
};

}  // namespace nemu

#endif
