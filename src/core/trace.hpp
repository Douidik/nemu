#ifndef NEMU_TRACE_HPP
#define NEMU_TRACE_HPP

#include "bus.hpp"
#include "cpu/disasm.hpp"

namespace nemu {

class Trace {
public:
  Trace(Bus *bus);
  Disasm disasm() const;

private:
  Disasm::Bytes disasm_bytes(const Disasm &disasm, size_t size) const;
  Disasm::Fetch disasm_fetch(const Disasm &disasm, Mode mode) const;
  Disasm::Operation disasm_operation(const Disasm &disasm, Mnemonic mnemonic) const;

  const Bus &m_bus;
};

}  // namespace nemu

#endif
