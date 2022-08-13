#ifndef NEMU_KEYBOARD_HPP
#define NEMU_KEYBOARD_HPP

#include "gamepad.hpp"
#include "keymap.hpp"
#include <array>
#include <span>

namespace nemu {

class Gamepad;
enum class State : uint32;

class Keyboard {
public:
  Keyboard(Keymap &keymap);
  void update(Gamepad (&gamepads)[2], State &state) const;

private:
  std::span<const uint8> keystate() const;

  Keymap &m_keymap;
  std::span<const uint8> m_keystate;
  std::array<std::pair<int32 &, GamepadButton>, 8> m_gamepad_map;
};

}  // namespace nemu

#endif
