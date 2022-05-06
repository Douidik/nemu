#ifndef NEMU_PPU_HPP
#define NEMU_PPU_HPP

#include "hardware.hpp"
#include "registers.hpp"

namespace nemu {

class Ppu : public Hardware<class Nes> {
public:
  Ppu(Nes *nes);

  void init() override;
  void tick() override;

  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_read(uint16 n);

  inline const auto &registers() const {
    return m_regs;
  }

private:
  void pre_render();
  void render();
  void post_render();

  bool use_latch();
  uint16 parse_vram_address(uint16 address) const;
  uint16 increment_vram_address();

  struct Pixel {
    uint8 index;
    bool enabled;
  };

  struct Pipeline {
    uint8 nametable;
    uint8 attribute;
    uint8 pattern_table[2];
  } m_pipeline;

  PpuRegisters m_regs;
  uint8 m_data_buffer;
  std::array<uint8, 2048> m_vram;
  std::array<uint8, 32> m_palette;
  uint32 m_scanline, m_cycle, m_framecount;
  bool m_latch;
};

}  // namespace nemu

#endif
