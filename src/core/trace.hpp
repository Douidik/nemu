#ifndef NEMU_TRACE_HPP
#define NEMU_TRACE_HPP

#include "bus.hpp"
#include "cpu/disasm.hpp"

namespace nemu {

class Trace {
public:
  Trace(const Bus *bus);
  cpu::Disasm disasm() const;

private:
  cpu::Disasm::Bytes disasm_bytes(const cpu::Disasm &disasm, size_t size) const;
  cpu::Disasm::Fetch disasm_fetch(const cpu::Disasm &disasm, cpu::Mode mode) const;
  cpu::Disasm::Operation disasm_operation(const cpu::Disasm &disasm, cpu::Mnemonic mnemonic) const;

  const Bus &m_bus;
};

}  // namespace nemu

#endif
