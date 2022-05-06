#ifndef NEMU_TRACE_HPP
#define NEMU_TRACE_HPP

namespace nemu {

class Bus;

class Trace {
public:
  Trace(Bus *bus);

private:
  Bus &m_bus;
};

}  // namespace nemu

#endif
