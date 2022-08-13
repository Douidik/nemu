#include "app.hpp"
#include "nes.hpp"
#include <SDL2/SDL_timer.h>
#include <chrono>
#include <fstream>
#include <iterator>
#include <thread>

namespace nemu {

constexpr uint64 FRAME_DELTA = 1000 / 60;
constexpr uint64 FRAME_TICKS = 341 * 260 / 3;

App::App(std::span<const char *> args) :
  m_window {m_user.window_info},
  m_keyboard {m_user.keymap},
  m_state {State::INIT},
  m_rom_data {parse_rom(args[1])},
  m_sdata {sdata::parse_file("assets/nemu.sd")},
  m_username {args[0]} {}

void App::run() {
  Rom rom {m_rom_data};
  Nes nes {rom};

  m_user = m_sdata.at(m_username);
  m_window.setup();
  m_renderer.setup(m_window);
  nes.init();

  uint32 timepoint_init = SDL_GetTicks();
  // Pre/Post frame time points, used to compute the deltatime
  uint32 timepoint[2] {};

  while (m_state != State::EXIT) {
    timepoint[0] = SDL_GetTicks();
    {
      for (uint64 n = 0; n < FRAME_TICKS; n++) {
        nes.tick();
      }

      uint64 time = std::max<uint64>(1, ((timepoint[0] - timepoint_init) / 1000));
      uint64 fps = nes.ppu().framecount() / time;

      m_keyboard.update(nes.gamepads(), m_state);
      m_window.update(m_state);
      m_renderer.draw(m_user.window_info, nes.ppu().canvas(), fps);
    }
    timepoint[1] = SDL_GetTicks();

    if (timepoint[1] - timepoint[0] < FRAME_DELTA) {
      SDL_Delay(FRAME_DELTA - (timepoint[1] - timepoint[0]));
    }

    std::swap(timepoint[0], timepoint[1]);
  }

  m_renderer.close();
  m_window.close();

  // Deserialize the user
  m_sdata[m_username] = sdata::Node {m_username, m_user};
  sdata::write_file("assets/nemu.sd", m_sdata);
}

std::vector<uint8> App::parse_rom(std::string_view path) const {
  std::ifstream fstream {&path[0], std::ios::binary};

  if (!fstream) {
    throw Exception {"Can't open rom file from: '{}'", path};
  }

  return {
    std::istreambuf_iterator<char>(fstream),
    {},
  };
}

}  // namespace nemu
