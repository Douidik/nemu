#include "app.hpp"
#include "exception.hpp"
#include <SDL2/SDL_timer.h>
#include <ctime>
#include <fstream>
#include <functional>

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

  m_window.map_input(m_keymap.gamepad.a, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_A);
  });
  m_window.map_input(m_keymap.gamepad.b, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_B);
  });
  m_window.map_input(m_keymap.gamepad.select, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_SELECT);
  });
  m_window.map_input(m_keymap.gamepad.start, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_START);
  });
  m_window.map_input(m_keymap.gamepad.up, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_UP);
  });
  m_window.map_input(m_keymap.gamepad.down, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_DOWN);
  });
  m_window.map_input(m_keymap.gamepad.left, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_LEFT);
  });
  m_window.map_input(m_keymap.gamepad.right, [&gamepad = m_nes.gamepad(0)] {
    gamepad.press_button(NES_GAMEPAD_RIGHT);
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

  m_trace_file = std::fopen(trace_filename().data(), "w");
  m_status = AppStatus::RUNNING;
  m_rom = Rom {parse_rom_data(m_args)};
  m_nes.init();

  uint32 cpu_instruction_counter {};
  uint32 ticks = SDL_GetTicks();

  while (m_status != AppStatus::EXITING) {
    trace_cpu(cpu_instruction_counter);

    m_window.process_events();
    m_window.process_inputs();
    m_nes.tick();

    // Cap the framerate to approximately 60 times per seconds (16ms delay)
    if (uint32 new_ticks = SDL_GetTicks(); (new_ticks - ticks) >= (1000 / 16)) {
      m_window.draw_canvas(m_nes.ppu().render_canvas());
      // Refresh frame ticks counter
      ticks = new_ticks;
    }
  }

  std::fclose(m_trace_file);
  m_window.end_context();
}

std::string App::trace_filename() const {
  return fmt::format("logs/nemu_{}.log", std::time(nullptr));
}

void App::trace_cpu(uint32 &instruction_counter) {
  constexpr size_t MAX_LINE_LENGTH {160};
  uint32 new_instruction_counter = m_nes.cpu().instruction_counter();

  // Trace the cpu status each time a new instruction is executed
  if (instruction_counter != new_instruction_counter) {
    fmt::print(m_trace_file, "{:{}}", m_nes.cpu().disasm(), MAX_LINE_LENGTH);
    // fmt::print(m_frame_file, "{}"

    // fmt::print(m_trace_file, "{} {}\n", registers, disasm), std::fflush(m_trace_file);
  }

  instruction_counter = new_instruction_counter;
}

}  // namespace nemu
