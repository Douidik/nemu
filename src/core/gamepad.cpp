#include "gamepad.hpp"
#include "nes.hpp"

namespace nemu {

Gamepad::Gamepad(Nes *nes) : Hardware {nes} {}

void Gamepad::init() {
  m_bits = 0x00, m_strobe = 0x00, m_mask = 0b0000'0001;
}

void Gamepad::tick() {}

uint8 Gamepad::press_button(GamepadButton input) {
  return m_bits |= input;
}

uint8 Gamepad::release_button(GamepadButton input) {
  return m_bits &= ~input;
}

uint8 Gamepad::cpu_write(uint16 n, uint8 data) {
  m_strobe = data;

  if (m_strobe & 0b1) {
    m_mask = 0b0000'0001;
  }

  return m_strobe;
}

uint8 Gamepad::cpu_read(uint16 n) {
  uint8 bit = 0b0;

  if ((m_mask <= 0b1000'0000) && (m_bits & m_mask)) {
    bit |= 0b1;
  }

  m_mask = m_mask << 1;

  if (m_strobe & 0b1) {
    m_mask = 0b0000'0001;
  }

  return bit;
}

}  // namespace nemu
