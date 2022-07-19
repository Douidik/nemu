#ifndef NEMU_PPU_HPP
#define NEMU_PPU_HPP

#include "canvas.hpp"
#include "hardware.hpp"
#include "registers.hpp"
#include <span>

namespace nemu {

class Ppu : public Hardware<class Nes> {
public:
  Ppu(Nes *nes);

  void init() override;
  void tick() override;

  Canvas render_canvas();
  
  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_read(uint16 n);

  inline const auto &registers() const {
    return m_regs;
  }

private:
  uint8 ppu_write(uint16 n, uint8 data);
  uint8 ppu_read(uint16 n);

  auto fetch_pattern(uint8 n);
  uint8 fetch_nametable(uint8 n);
  uint8 fetch_attribute(uint8 n);

  uint8 render_background(uint16 x, uint16 y);
  uint8 render_foreground(uint16 x, uint16 y);

  uint16 parse_vram_address(uint16 address);
  uint16 increment_vram_address();

  PpuRegisters m_regs;
  bool m_latch;
  uint8 m_data_buffer;
  std::array<uint8, 2048> m_vram;
  std::array<uint8, 32> m_palette;
  uint32 m_scanline, m_ticks, m_frame_count;
};

}  // namespace nemu

#endif
