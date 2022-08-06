#include "app.hpp"
#include "exception.hpp"
#include "trace.hpp"
#include <SDL2/SDL_timer.h>
#include <ctime>
#include <fstream>
#include <functional>
#include <limits>

namespace nemu {

App::App(std::span<const char *> args) :
  m_nes {m_rom},
  m_window {800, 800, "NEMU"},
  m_status {AppStatus::INITIALIZED},
  m_args {args} {
  m_keymap = sdata::parse_file("assets/keymap.sd");

  m_window.map_event(SDL_QUIT, [this](const SDL_Event &) {
    m_status = AppStatus::EXITING;
  });
  m_window.map_input(m_keymap.app.exit, [this] {
    m_status = AppStatus::EXITING;
  });

  const std::unordered_map<uint32, GamepadButton> gamepad_keymap = {
    {m_keymap.gamepad.a, NES_GAMEPAD_A},
    {m_keymap.gamepad.b, NES_GAMEPAD_B},
    {m_keymap.gamepad.select, NES_GAMEPAD_SELECT},
    {m_keymap.gamepad.start, NES_GAMEPAD_START},
    {m_keymap.gamepad.up, NES_GAMEPAD_UP},
    {m_keymap.gamepad.down, NES_GAMEPAD_DOWN},
    {m_keymap.gamepad.left, NES_GAMEPAD_LEFT},
    {m_keymap.gamepad.right, NES_GAMEPAD_RIGHT},
  };

  for (const auto &[key, button] : gamepad_keymap) {
    m_window.map_input(
      key,
      [&gamepad = m_nes.gamepad(0), button = button] {
        gamepad.press_button(button);
      },
      [&gamepad = m_nes.gamepad(0), button = button] {
        gamepad.release_button(button);
      });
  }
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

  m_trace_file = std::fopen(trace_filename().data(), "w");
  m_status = AppStatus::RUNNING;
  m_rom = Rom {parse_rom_data(m_args)};
  m_nes.init();

  uint32 cpu_instruction_counter {};
  uint32 nes_ticks = SDL_GetTicks();
  uint32 app_ticks = SDL_GetTicks();

  // f64 upms = 1.0 / m_nes.frequency<std::milli>();
  // f64 upms = 1000.0 / NES_FREQUENCY_HZ;
  // TODO: upms
  f64 upms {};
  f64 fpms = 1000.0 / 60.0;

  while (m_status != AppStatus::EXITING) {
    m_nes.tick();
    
    // if (uint32 new_ticks = SDL_GetTicks(); (new_ticks - nes_ticks) >= upms) {
    //   m_nes.tick();

    //   // Refresh nes ticks counter
    //   nes_ticks = new_ticks;
    // }

    if (uint32 new_ticks = SDL_GetTicks(); (new_ticks - app_ticks) >= fpms) {
      m_window.process_events();
      m_window.process_inputs();
      m_window.draw_canvas(m_nes.ppu().render_canvas());

      // Refresh frame ticks counter
      app_ticks = new_ticks;
    }

    // trace_cpu(cpu_instruction_counter);
  }

  std::fclose(m_trace_file);
  m_window.end_context();
}

std::string App::trace_filename() const {
  return fmt::format("logs/nemu_{}.log", std::time(nullptr));
}

void App::trace_cpu(uint32 &instruction_counter) {
  uint32 new_instruction_counter = m_nes.cpu().instruction_counter();

  // Trace the cpu status each time a new instruction is executed
  if (instruction_counter != new_instruction_counter) {
    fmt::print(m_trace_file, "{:(160)}\n", Trace {&m_nes}.disasm());
    std::fflush(m_trace_file);
  }

  instruction_counter = new_instruction_counter;
}

}  // namespace nemu
