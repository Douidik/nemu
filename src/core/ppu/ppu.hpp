#ifndef NEMU_PPU_HPP
#define NEMU_PPU_HPP

#include "hardware.hpp"
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

  std::optional<uint8> cpu_peek(uint16 n) const;
  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_read(uint16 n);

  inline const auto &regsiters() const {
    return m_regs;
  }

  inline std::string_view timing() const {
    return m_timing;
  }

  Canvas render_canvas();

private:
  std::optional<uint8> ppu_peek() const;
  uint8 ppu_write(uint8 data);
  uint8 ppu_read();

  uint16 ppu_address();
  uint16 vram_address(uint16 n) const;
  uint16 color_address(uint16 n) const;

  Canvas render_background();
  Canvas render_foreground();

  PpuRegisters m_regs;

  std::array<uint8, 2048> m_vram;
  std::array<uint8, 32> m_colors;

  uint32 m_scanline, m_ticks, m_framecount;
  std::string_view m_timing;
};

}  // namespace nemu

#endif
