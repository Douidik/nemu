#ifndef NEMU_APP_HPP
#define NEMU_APP_HPP

#include "keymap.hpp"
#include "nes.hpp"
#include "window.hpp"
#include <vector>

namespace nemu {

enum class AppStatus {
  INITIALIZED,
  RUNNING,
  PAUSED,
  EXITING,
};

class App {
public:
  App(std::span<const char *> args);
  void run();

private:
  std::string trace_filename() const;
  void trace_cpu(const Nes &nes, uint32 &instruction_counter);
  std::vector<uint8> read_rom(std::span<const char *> args);

  Keymap m_keymap;
  Window m_window;
  std::span<const char *> m_args;
  AppStatus m_status;
  std::FILE *m_trace_file;
};

}  // namespace nemu

#endif
