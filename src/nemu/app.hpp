#ifndef NEMU_APP_HPP
#define NEMU_APP_HPP

#include "keyboard.hpp"
#include "renderer.hpp"
#include "user.hpp"
#include "window.hpp"
#include <span>

namespace nemu {

enum class State : uint32 {
  INIT,
  RUN,
  PAUSE,
  EXIT,
};

class App {
public:
  App(std::span<const char *> args);
  void run();
  
private:
  std::vector<uint8> parse_rom(std::string_view path) const;

  State m_state;
  User m_user;
  Window m_window;
  Renderer m_renderer;
  Keyboard m_keyboard;
  std::vector<uint8> m_rom_data;

  sdata::Node m_sdata;
  std::string_view m_username;
};

}  // namespace nemu

#endif
