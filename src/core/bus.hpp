#ifndef NEMU_BUS_HPP
#define NEMU_BUS_HPP

#include "cpu/cpu.hpp"
#include <array>

namespace nemu {

class Bus {
public:
  Bus() : m_cpu {this} {}

  virtual void init() = 0;
  virtual void tick() = 0;

  virtual std::optional<uint8> cpu_peek(uint16 n) const = 0;

  virtual uint8 cpu_write(uint16 n, uint8 data) = 0;
  virtual uint8 cpu_read(uint16 n) = 0;

  inline auto &ram() {
    return m_ram;
  }

  inline Cpu &cpu() {
    return m_cpu;
  }

protected:
  Cpu m_cpu;
  std::array<uint8, 2048> m_ram;
};

}  // namespace nemu

#endif
