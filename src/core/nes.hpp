#ifndef NEMU_NES_HPP
#define NEMU_NES_HPP

#include "bus.hpp"
#include "gamepad.hpp"
#include "ppu/dma.hpp"
#include "ppu/ppu.hpp"
#include "rom.hpp"

namespace nemu {

class Nes : public Bus {
public:
  Nes(Rom &rom);

  void init() override;
  void tick() override;

  uint8 cpu_write(uint16 n, uint8 data) override;
  uint8 cpu_peek(uint16 n) const override;
  uint8 cpu_read(uint16 n) override;

  uint8 ppu_write(uint16 n, uint8 data);
  uint8 ppu_peek(uint16 n) const;
  uint8 ppu_read(uint16 n);

  inline auto &gamepads() {
    return m_gamepads;
  }

  inline Ppu &ppu() {
    return m_ppu;
  }

  inline auto mapper() const {
    return m_mapper;
  }

  inline auto zip() {
    return std::forward_as_tuple(m_cpu, m_ram, m_ppu, m_gamepads);
  }

private:
  Ppu m_ppu;
  Gamepad m_gamepads[2];
  std::shared_ptr<class Mapper> m_mapper;
  std::optional<ppu::Dma> m_dma;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

// template<>
// struct Serializer<Nes> : Scheme<Nes(Cpu, std::array<uint8, 2048>, Ppu, std::array<Gamepad, 2>)> {
//   Map map(Nes &nes) {
//     auto [cpu, ram, ppu, gamepads] = nes.zip();

//     return {
//       {"cpu", cpu},
//       {"ram", ram},
//       {"ppu", ppu},
//       {"gamepads", gamepads},
//     };
//   }
// };

}  // namespace sdata

#endif
