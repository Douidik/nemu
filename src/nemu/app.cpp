#include "app.hpp"
#include "exception.hpp"
#include "trace.hpp"
#include <chrono>
#include <fstream>
#include <functional>
#include <thread>

namespace nemu {

App::App(std::span<const char *> args) :
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
}

std::vector<uint8> App::read_rom(std::span<const char *> args) {
  if (args.size() < 2) {
    throw nemu::Exception {"Rom path unspecified"};
  }

  std::ifstream fstream {args[1], std::ios::binary};

  if (!fstream.is_open()) {
    throw nemu::Exception {"Can't open rom from: '{}'", args[1]};
  }

  return {std::istreambuf_iterator(fstream), {}};
}

void App::run() {
  //m_trace_file = std::fopen(trace_filename().data(), "w+");
  m_status = AppStatus::RUNNING;

  auto rom_data = read_rom(m_args);
  Rom rom = Rom {rom_data};

  Nes nes {rom};
  nes.init();

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
      [&gamepad = nes.gamepad(0), button = button] {
        gamepad.press_button(button);
      },
      [&gamepad = nes.gamepad(0), button = button] {
        gamepad.release_button(button);
      });
  }

  uint32 cpu_instruction_counter {};
  auto time = std::chrono::system_clock::now();

  constexpr uint64 FRAME_DELTA = 1000 / 60;
  // constexpr uint64 FRAME_TICKS = 29780;
  constexpr uint64 FRAME_TICKS = Canvas::W * Canvas::H / 3;

  m_window.run_context();

  while (m_status != AppStatus::EXITING) {

    for (uint64 n = 0; n < FRAME_TICKS; n++) {
      // trace_cpu(nes, cpu_instruction_counter);
      nes.tick();
    }

    m_window.process_events();
    m_window.process_inputs();
    m_window.draw_canvas(nes.ppu().canvas());

    auto new_time = std::chrono::system_clock::now();
    auto sleep_time = std::chrono::duration<double, std::milli>(new_time - time);

    if (sleep_time.count() < FRAME_DELTA) {
      std::this_thread::sleep_for(sleep_time);
    }

    time = new_time;
  }

  //std::fclose(m_trace_file);
  m_window.end_context();
}

std::string App::trace_filename() const {
  return fmt::format("logs/nemu_{}.log", std::time(nullptr));
}

void App::trace_cpu(const Nes &nes, uint32 &instruction_counter) {
  uint32 new_instruction_counter = nes.cpu().instruction_counter();

  // Trace the cpu status each time a new instruction is executed
  if (instruction_counter != new_instruction_counter) {
    fmt::print(m_trace_file, "{:(160)}\n", Trace {&nes}.disasm()), std::fflush(m_trace_file);
  }

  instruction_counter = new_instruction_counter;
}

}  // namespace nemu
