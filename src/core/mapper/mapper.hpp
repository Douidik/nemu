#ifndef NEMU_MAPPER_HPP
#define NEMU_MAPPER_HPP

#include "int.hpp"
#include "rom.hpp"
#include <memory>
#include <span>

namespace nemu {

class Mapper {
public:
  Mapper(Rom &rom) : m_rom {rom} {}

  virtual void init() {}

  static std::shared_ptr<Mapper> create(Rom &rom);

  virtual Mirror mirror() const = 0;

  virtual uint8 *cpu_write(uint16 n, uint8 data) = 0;
  virtual const uint8 *cpu_peek(uint16 n) const = 0;
  virtual uint8 *cpu_read(uint16 n) = 0;

  virtual uint8 *ppu_write(uint16 n, uint8 data) = 0;
  virtual const uint8 *ppu_peek(uint16 n) const = 0;
  virtual uint8 *ppu_read(uint16 n) = 0;

  virtual std::span<const uint8> pattern(uint8 n) const = 0;

protected:
  Rom &m_rom;
};

}  // namespace nemu

#endif
