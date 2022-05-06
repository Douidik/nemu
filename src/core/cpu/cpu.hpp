#ifndef NEMU_CPU_HPP
#define NEMU_CPU_HPP

#include "hardware.hpp"
#include "instructions.hpp"
#include "interrupt.hpp"
#include "registers.hpp"
#include <optional>

namespace nemu {

class Cpu : public Hardware<class Bus> {
public:
  Cpu(Bus *bus);

  void init() override;
  void tick() override;

  void irq();
  void nmi();

  inline const auto &registers() const {
    return m_regs;
  }

  inline uint32 cycles_remaining() const {
    return m_cycles_remaining;
  }

  inline uint32 cycles_counter() const {
    return m_cycles_counter;
  }

private:
  uint16 interrupt(Interrupt interrupt, uint16 pc);

  void parse_instruction(Instruction instruction);
  std::optional<uint8> execute_operation(Instruction instruction, uint8 operand);

  uint16 read_operand_address(Instruction instruction);
  uint8 parse_operand(uint8 operand);
  uint8 add_with_carry(uint8 operand);
  uint8 bitwise_fn(uint8 operand, uint8 (*fn)(uint8 a, uint8 b));
  void branch(bool condition, uint8 offset);
  void compare(uint8 a, uint8 b);

  uint8 stack_push(uint8 data);
  uint8 stack_pop();

  CpuRegisters m_regs;
  uint32 m_cycles_remaining, m_cycles_counter;
};

}  // namespace nemu

#endif
