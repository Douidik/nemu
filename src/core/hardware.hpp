#ifndef NEMU_HARDWARE_HPP
#define NEMU_HARDWARE_HPP

#include "int.hpp"

namespace nemu {

template<typename T>
class Hardware {
public:
  Hardware(T *bus) : m_bus {*bus} {}

  virtual void init() = 0;
  virtual void tick() = 0;

protected:
  T &m_bus;
};

}  // namespace nemu

#endif
