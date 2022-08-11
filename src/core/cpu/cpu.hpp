#ifndef NEMU_CPU_HPP
#define NEMU_CPU_HPP

#include "hardware.hpp"
#include "instructions.hpp"
#include "interrupt.hpp"
#include "registers.hpp"
#include <optional>

namespace nemu {

// TODO: create functions for shift operations (ASL, LSR, ROR, ROL)

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

  inline uint32 instruction_counter() const {
    return m_instruction_counter;
  }

  inline uint16 program_counter() const {
    return m_regs.pc;
  }

  inline auto zip() {
    return std::forward_as_tuple(m_regs, m_cycles_remaining, m_instruction_counter);
  }

private:
  uint16 interrupt(Interrupt interrupt, uint16 pc);

  void parse_instruction(cpu::Instruction instruction);
  uint16 parse_address(cpu::Instruction instruction);
  auto execute_operation(cpu::Instruction instruction, uint16 operand) -> std::optional<uint8>;

  uint8 parse_operand(uint8 operand);
  uint8 add_with_carry(uint8 operand);
  uint8 bitwise_fn(uint8 operand, uint8 (*fn)(uint8 a, uint8 b));
  void branch(bool condition, uint8 offset);
  void compare(uint8 a, uint8 b);

  uint8 stack_push(uint8 data);
  uint8 stack_pop();

  cpu::Registers m_regs;
  uint32 m_cycles_remaining;
  uint32 m_instruction_counter;
};

}  // namespace nemu

namespace sdata {
  
using namespace nemu;
using namespace nemu::cpu;

template<>
struct Serializer<Cpu> : Scheme<Cpu(Registers, uint32, uint32)> {
  Map map(Cpu &cpu) {
    auto [registers, cycles_remaining, instruction_counter] = cpu.zip();

    return {
      {"registers", registers},
      {"cycles_remaining", cycles_remaining},
      {"instruction_counter", instruction_counter},
    };
  }
};

}  // namespace sdata

#endif
