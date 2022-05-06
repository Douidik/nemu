#include "trace.hpp"
#include "bus.hpp"

namespace nemu {

Trace::Trace(Bus *bus) : m_bus {*bus} {}

}  // namespace nemu
