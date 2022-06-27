#ifndef NEMU_APP_HPP
#define NEMU_APP_HPP

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
  void trace_cpu(uint32 &instruction_counter);
  std::span<uint8> parse_rom_data(std::span<const char *> args);

  Rom m_rom;
  Nes m_nes;
  Window m_window;
  std::span<const char *> m_args;
  std::vector<uint8> m_rom_data;
  AppStatus m_status;
  FILE *m_trace_file;
};

}  // namespace nemu

#endif
