#ifndef NEMU_CPU_HPP
#define NEMU_CPU_HPP

#include "disasm.hpp"
#include "hardware.hpp"
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

  inline const auto &disasm() const {
    return m_disasm;
  }

  inline const auto &registers() const {
    return m_regs;
  }

  inline uint32 cycles_remaining() const {
    return m_cycles_remaining;
  }

  inline uint32 instruction_counter() const {
    return m_instruction_counter;
  }

private:
  uint16 interrupt(Interrupt interrupt, uint16 pc);

  void parse_instruction(Instruction instruction);
  auto execute_operation(Instruction instruction, uint16 operand) -> std::optional<uint8>;

  uint16 read_operand_address(Instruction instruction);
  uint8 parse_operand(uint8 operand);
  uint8 add_with_carry(uint8 operand);
  uint8 bitwise_fn(uint8 operand, uint8 (*fn)(uint8 a, uint8 b));
  void branch(bool condition, uint8 offset);
  void compare(uint8 a, uint8 b);

  uint8 stack_push(uint8 data);
  uint8 stack_pop();

  Disasm m_disasm;
  CpuRegisters m_regs;
  uint32 m_cycles_remaining, m_instruction_counter;
};

}  // namespace nemu

#endif
