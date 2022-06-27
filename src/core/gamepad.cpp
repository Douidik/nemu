#include "gamepad.hpp"
#include "nes.hpp"

namespace nemu {

Gamepad::Gamepad(Nes *nes) : Hardware {nes} {}

void Gamepad::init() {
  m_bits = 0xFF;
}

void Gamepad::tick() {
  m_bits = 0xFF;
}

uint8 Gamepad::press_button(GamepadInput input) {
  return m_bits &= ~input;
}

}  // namespace nemu
