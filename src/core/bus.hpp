#ifndef NEMU_BUS_HPP
#define NEMU_BUS_HPP

#include "cpu/cpu.hpp"
#include "trace.hpp"
#include <array>

namespace nemu {

class Bus {
public:
  Bus() : m_cpu {this}, m_trace {this} {}

  virtual void tick() = 0;

  virtual uint8 cpu_write(uint16 n, uint8 data) = 0;
  virtual uint8 cpu_read(uint16 n) = 0;

  virtual uint8 ppu_write(uint16 n, uint8 data) = 0;
  virtual uint8 ppu_read(uint16 n) = 0;

  virtual uint8 apu_write(uint16 n, uint8 data) = 0;
  virtual uint8 apu_read(uint16 n) = 0;

  auto &ram() {
    return m_ram;
  }

  Cpu &cpu() {
    return m_cpu;
  }

  Trace &trace() {
    return m_trace;
  }

protected:
  Trace m_trace;
  Cpu m_cpu;
  std::array<uint8, 2048> m_ram;
};

}  // namespace nemu

#endif
