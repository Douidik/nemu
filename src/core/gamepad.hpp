#ifndef NEMU_GAMEPAD_HPP
#define NEMU_GAMEPAD_HPP

#include "hardware.hpp"
#include <sdata.hpp>

namespace nemu {

enum GamepadButton : uint8 {
  NES_GAMEPAD_A = 1 << 0,
  NES_GAMEPAD_B = 1 << 1,
  NES_GAMEPAD_SELECT = 1 << 2,
  NES_GAMEPAD_START = 1 << 3,
  NES_GAMEPAD_UP = 1 << 4,
  NES_GAMEPAD_DOWN = 1 << 5,
  NES_GAMEPAD_LEFT = 1 << 6,
  NES_GAMEPAD_RIGHT = 1 << 7,
};

class Gamepad : Hardware<class Nes> {
public:
  Gamepad(class Nes *nes);

  void init() override;
  void tick() override;

  uint8 press_button(GamepadButton input);
  uint8 release_button(GamepadButton input);

  inline uint8 bits() const {
    return m_bits;
  }

  uint8 cpu_write(uint16 n, uint8 data);
  uint8 cpu_read(uint16 n);

  inline auto zip() {
    return std::forward_as_tuple(m_bits, m_strobe, m_mask);
  }

private:
  uint8 m_bits, m_strobe;
  uint16 m_mask;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<Gamepad> : Scheme<Gamepad(uint8, uint8, uint16)> {
  Map map(Gamepad &gamepad) {
    auto [bits, strobe, mask] = gamepad.zip();

    return {
      {"bits", bits},
      {"strobe", strobe},
      {"mask", mask},
    };
  }
};

}  // namespace sdata

#endif
