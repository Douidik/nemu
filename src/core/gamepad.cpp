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
  return m_bits |= input;
}

uint8 Gamepad::cpu_write(uint16 n, uint8 data) {
  m_strobe = data & 0x01, m_shift = 0x00;
  return {};
}

uint8 Gamepad::cpu_read(uint16 n) {
  if (!m_shift || m_strobe) {
    return 0x01;
  }

  return m_bits & (1 << m_shift++);
}

}  // namespace nemu
