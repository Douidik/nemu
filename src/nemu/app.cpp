#include "app.hpp"
#include "exception.hpp"
#include <SDL2/SDL_timer.h>
#include <ctime>
#include <fstream>
#include <functional>
#include <iterator>

namespace nemu {

App::App(std::span<const char *> args) :
  m_nes {m_rom},
  m_args {args},
  m_window {800, 800, "NEMU"},
  m_status {AppStatus::INITIALIZED} {
  // Register window callbacks:

  m_window.map_event(SDL_QUIT, [this](const SDL_Event &) {
    m_status = AppStatus::EXITING;
  });

  m_window.map_input('x', [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_A);
  });

  m_window.map_input('c', [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_B);
  });

  m_window.map_input(SDLK_KP_ENTER, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_START);
  });

  m_window.map_input(SDLK_TAB, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_SELECT);
  });

  m_window.map_input(SDLK_UP, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_UP);
  });

  m_window.map_input(SDLK_DOWN, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_UP);
  });

  m_window.map_input(SDLK_LEFT, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_UP);
  });

  m_window.map_input(SDLK_RIGHT, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_UP);
  });

  m_window.map_input(SDLK_ESCAPE, [this] {
    m_status = AppStatus::EXITING;
  });
}

std::span<uint8> App::parse_rom_data(std::span<const char *> args) {
  if (args.size() < 2) {
    throw nemu::Exception {"Missing rom path argument"};
  }

  std::ifstream fstream {args[1], std::ios::binary};

  if (!fstream.is_open()) {
    throw nemu::Exception {"Can't open rom from: '{}'", args[1]};
  }

  return m_rom_data = {std::istreambuf_iterator(fstream), {}};
}

void App::run() {
  m_window.run_context();

  m_trace_file = fopen(trace_filename().data(), "w");
  m_status = AppStatus::RUNNING;
  m_rom = Rom {parse_rom_data(m_args)};
  m_nes.init();

  uint32 cpu_instruction_counter {};
  uint32 ticks = SDL_GetTicks();

  while (m_status != AppStatus::EXITING) {
    m_nes.tick();

    m_window.process_events();
    // m_window.process_inputs();

    // Cap the framerate to approximately 60 times per seconds (16ms delay)
    if (uint32 new_ticks = SDL_GetTicks(); (new_ticks - ticks) >= (1000 / 10)) {
      m_window.draw_canvas(m_nes.ppu().render_canvas());
      // Refresh frame ticks counter
      ticks = new_ticks;
    }

    trace_cpu(cpu_instruction_counter);
  }

  fclose(m_trace_file);
  m_window.end_context();
}

std::string App::trace_filename() const {
  return fmt::format("logs/trace_{}.nemu", std::time(0));
}

void App::trace_cpu(uint32 &instruction_counter) {
  uint32 new_instruction_counter = m_nes.cpu().instruction_counter();

  const auto &registers = m_nes.cpu().registers();
  const auto &disasm = m_nes.cpu().disasm();

  // Trace the cpu status each time a new instruction is executed
  if (instruction_counter != new_instruction_counter) {
    fmt::print(m_trace_file, "{} {}\n", registers, disasm);
  }

  instruction_counter = new_instruction_counter;
}

}  // namespace nemu
