#include "keyboard.hpp"
#include "SDL2/SDL.h"
#include "app.hpp"

namespace nemu {

Keyboard::Keyboard(Keymap &keymap) :
  m_keymap {keymap},
  m_keystate {keystate()},
  m_gamepad_map {{
    {m_keymap.gamepad.a, NES_GAMEPAD_A},
    {m_keymap.gamepad.b, NES_GAMEPAD_B},
    {m_keymap.gamepad.select, NES_GAMEPAD_SELECT},
    {m_keymap.gamepad.start, NES_GAMEPAD_START},
    {m_keymap.gamepad.up, NES_GAMEPAD_UP},
    {m_keymap.gamepad.down, NES_GAMEPAD_DOWN},
    {m_keymap.gamepad.left, NES_GAMEPAD_LEFT},
    {m_keymap.gamepad.right, NES_GAMEPAD_RIGHT},
  }} {}

void Keyboard::update(Gamepad (&gamepads)[2], State &state) const {
  for (auto &[key, button] : m_gamepad_map) {
    if (m_keystate[key]) {
      gamepads[0].press_button(button);
    } else {
      gamepads[0].release_button(button);
    }
  }

  if (m_keystate[m_keymap.app.exit]) {
    state = State::EXIT;
  }

  if (m_keystate[m_keymap.app.pause]) {
    state = State::PAUSE;
  }
}

std::span<const uint8> Keyboard::keystate() const {
  if (!SDL_WasInit(SDL_INIT_EVENTS)) {
    throw Exception {"SDL must initilalize SDL_INIT_EVENTS to setup the keyboard"};
  }

  int32 keycount;
  auto *keystate = SDL_GetKeyboardState(&keycount);

  return {keystate, static_cast<size_t>(keycount)};
}

}  // namespace nemu
