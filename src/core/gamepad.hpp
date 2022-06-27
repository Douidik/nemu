#ifndef NEMU_GAMEPAD_HPP
#define NEMU_GAMEPAD_HPP

#include "hardware.hpp"
#include "int.hpp"

namespace nemu {

enum GamepadInput : uint8 {
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

  uint8 press_button(GamepadInput input);

  inline uint8 bits() const {
    return m_bits;
  }

private:
  uint8 m_bits;
};

}  // namespace nemu

#endif
