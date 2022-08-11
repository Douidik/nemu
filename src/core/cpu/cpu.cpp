#include "cpu.hpp"
#include "bus.hpp"
#include "exception.hpp"
#include <optional>

namespace nemu {

using namespace cpu;

Cpu::Cpu(Bus *bus) : Hardware {bus} {}

void Cpu::init() {
  m_bus.ram() = {}, m_cycles_remaining = 0, m_instruction_counter = 0;

  m_regs = Registers {
    .status = {},
    .a = 0x00,
    .x = 0x00,
    .y = 0x00,
    .sp = 0xFD,
    .pc = 0x00,
  };

  m_regs.pc = interrupt(CPU_RST, m_regs.pc);
}

void Cpu::tick() {
  if (m_cycles_remaining-- < 1) {
    uint8 opcode = m_bus.cpu_read(m_regs.pc++);
    auto instruction = INSTRUCTION_SET[opcode];
    parse_instruction(instruction);
    m_instruction_counter++;
  }
}

void Cpu::irq() {
  if (!m_regs.status.i) {
    m_regs.pc = interrupt(CPU_IRQ, m_regs.pc);
  }
}

void Cpu::nmi() {
  m_regs.pc = interrupt(CPU_NMI, m_regs.pc);
}

uint16 Cpu::interrupt(Interrupt interrupt, uint16 pc) {
  m_regs.status.b = 1;

  if (interrupt.push & Interrupt::PUSH_PC) {
    stack_push(m_regs.pc >> 8);
    stack_push(m_regs.pc & 0xFF);
  }

  if (interrupt.push & Interrupt::PUSH_STATUS) {
    stack_push(m_regs.status.bits);
  }

  m_cycles_remaining += interrupt.cycles;
  m_regs.status.bits |= interrupt.status_mask;

  return m_bus.cpu_read(interrupt.vector + 1) << 8 | m_bus.cpu_read(interrupt.vector);
}

void Cpu::parse_instruction(Instruction instruction) {
  if (instruction.mode & ACC) {
    auto output = execute_operation(instruction, m_regs.a);

    if (output.has_value()) {
      m_regs.a = output.value();
    }

    // TODO: update when available:
    // execute_operation(instruction, m_regs.a).and_then([&](uint8 output) {
    //   m_regs.a = output;
    // });
  }

  if (instruction.mode & IMM) {
    execute_operation(instruction, m_bus.cpu_read(m_regs.pc++));
  }

  if (instruction.mode & IMP) {
    execute_operation(instruction, {});
  }

  if (instruction.mode & REL) {
    uint16 offset = m_bus.cpu_read(m_regs.pc++);

    if (offset & 0x80) {
      offset |= 0xFF00;
    }

    execute_operation(instruction, offset);
  }

  if (instruction.mode & MEMORY) {
    uint16 address = parse_address(instruction);

    if (instruction.mnemonic & JUMP) {
      execute_operation(instruction, address);
    }

    else if (instruction.mnemonic & STORE) {
      m_bus.cpu_write(address, *execute_operation(instruction, {}));
    }

    else {
      auto output = execute_operation(instruction, m_bus.cpu_read(address));

      if (output.has_value()) {
        m_bus.cpu_write(address, *output);
      }

      // TODO: update when available:
      // execute_operation(instruction, m_bus.cpu_read(address)).and_then([&](uint8 output) {
      //   m_bus.cpu_write(address, output);
      // });
    }
  }

  m_cycles_remaining += instruction.cycles;
}

uint16 Cpu::parse_address(Instruction instruction) {
  const auto absolute = [&](uint8 offset) -> uint16 {
    uint8 address_bytes[] = {
      m_bus.cpu_read(m_regs.pc++),
      m_bus.cpu_read(m_regs.pc++),
    };

    return (address_bytes[1] << 8 | address_bytes[0]) + offset;
  };

  const auto zero_page = [&](uint8 offset) -> uint8 {
    return (m_bus.cpu_read(m_regs.pc++) + offset) & 0xFF;
  };

  switch (instruction.mode) {
  case ABS: return absolute({});
  case ABX: return absolute(m_regs.x);
  case ABY: return absolute(m_regs.y);

  case ZER: return zero_page({});
  case ZPX: return zero_page(m_regs.x);
  case ZPY: return zero_page(m_regs.y);

  case IND: {
    uint8 address_bytes[] = {
      m_bus.cpu_read(m_regs.pc++),
      m_bus.cpu_read(m_regs.pc++),
    };

    uint16 address = (address_bytes[1] << 8) | address_bytes[0];

    uint8 destination_bytes[] = {
      m_bus.cpu_read(address),
      // 6502 page boundary bug emulation
      address_bytes[0] != 0xFF ? m_bus.cpu_read(address + 1) : m_bus.cpu_read(address & 0xFF00),
    };

    return destination_bytes[1] << 8 | destination_bytes[0];
  }

  case IDX: {
    uint8 address = m_bus.cpu_read(m_regs.pc++) + m_regs.x;

    uint8 destination_bytes[] = {
      m_bus.cpu_read(address++ & 0xFF),
      m_bus.cpu_read(address++ & 0xFF),
    };

    return destination_bytes[1] << 8 | destination_bytes[0];
  }

  case IDY: {
    uint8 address = m_bus.cpu_read(m_regs.pc++);

    uint8 address_bytes[] = {
      m_bus.cpu_read(address++ & 0xFF),
      m_bus.cpu_read(address++ & 0xFF),
    };

    uint16 destination = (address_bytes[1] << 8 | address_bytes[0]) + m_regs.y;

    if ((destination & 0xFF00) != address_bytes[1] << 8) {
      m_cycles_remaining++;
    }

    return destination;
  }

  default: return {};
  }
}

auto Cpu::execute_operation(Instruction instruction, uint16 operand) -> std::optional<uint8> {
  switch (instruction.mnemonic) {
  case ADC: {
    m_regs.a = add_with_carry(operand);
  } break;

  case AND: {
    m_regs.a = bitwise_fn(operand, [](uint8 a, uint8 b) -> uint8 {
      return a & b;
    });
  } break;

  case ASL: {
    uint8 output = operand << 1;

    m_regs.status.c = operand & 0x80;
    m_regs.status.n = output & 0x80;
    m_regs.status.z = output == 0x00;

    return output;
  }

  case BCC: {
    branch(!m_regs.status.c, operand);
  } break;

  case BCS: {
    branch(m_regs.status.c, operand);
  } break;

  case BEQ: {
    branch(m_regs.status.z, operand);
  } break;

  case BIT: {
    m_regs.status.n = operand & 0x80;
    m_regs.status.v = operand & 0x40;
    m_regs.status.z = !(m_regs.a & operand);
  } break;

  case BMI: {
    branch(m_regs.status.n, operand);
  } break;

  case BNE: {
    branch(!m_regs.status.z, operand);
  } break;

  case BPL: {
    branch(!m_regs.status.n, operand);
  } break;

  case BRK: {
    m_regs.pc = interrupt(CPU_BRK, m_regs.pc + 1);
  } break;

  case BVC: {
    branch(!m_regs.status.v, operand);
  } break;

  case BVS: {
    branch(m_regs.status.v, operand);
  } break;

  case CLC: {
    m_regs.status.c = 0;
  } break;

  case CLD: {
    m_regs.status.d = 0;
  } break;

  case CLI: {
    m_regs.status.i = 0;
  } break;

  case CLV: {
    m_regs.status.v = 0;
  } break;

  case CMP: {
    compare(m_regs.a, operand);
  } break;

  case CPX: {
    compare(m_regs.x, operand);
  } break;

  case CPY: {
    compare(m_regs.y, operand);
  } break;

  case DEC: {
    return parse_operand((operand - 1) % 256);
  }

  case DEX: {
    m_regs.x = parse_operand((m_regs.x - 1) % 256);
  } break;

  case DEY: {
    m_regs.y = parse_operand((m_regs.y - 1) % 256);
  } break;

  case EOR: {
    m_regs.a = bitwise_fn(operand, [](uint8 a, uint8 b) -> uint8 {
      return a ^ b;
    });
  } break;

  case INC: {
    return parse_operand((operand + 1) % 256);
  }

  case INX: {
    m_regs.x = parse_operand((m_regs.x + 1) % 256);
  } break;

  case INY: {
    m_regs.y = parse_operand((m_regs.y + 1) % 256);
  } break;

  case JMP: {
    m_regs.pc = operand;
  } break;

  case JSR: {
    stack_push((m_regs.pc - 1) >> 8);
    stack_push((m_regs.pc - 1) & 0xFF);

    m_regs.pc = operand;
  } break;

  case LDA: {
    m_regs.a = parse_operand(operand);
  } break;

  case LDX: {
    m_regs.x = parse_operand(operand);
  } break;

  case LDY: {
    m_regs.y = parse_operand(operand);
  } break;

  case LSR: {
    uint8 output = operand >> 1;

    m_regs.status.c = operand & 0x01;
    m_regs.status.n = output & 0x80;
    m_regs.status.z = output == 0x00;

    return output;
  }

  case ORA: {
    m_regs.a = bitwise_fn(operand, [](uint8 a, uint8 b) -> uint8 {
      return a | b;
    });
  } break;

  case PHA: {
    stack_push(m_regs.a);
  } break;

  case PHP: {
    stack_push(m_regs.status.bits | (0b0011'0000));
  } break;

  case PLA: {
    m_regs.a = parse_operand(stack_pop());
  } break;

  case PLP: {
    m_regs.status.bits = stack_pop() | (m_regs.status.bits & 0b0011'0000);
  } break;

  case ROL: {
    uint16 output = (operand << 1) | m_regs.status.c;

    m_regs.status.c = output > 0xFF;
    m_regs.status.n = output & 0x80;
    m_regs.status.z = output == 0x00;

    return output & 0xFF;
  }

  case ROR: {
    uint16 output = (operand >> 1) | (m_regs.status.c << 7);

    m_regs.status.c = operand & 0x01;
    m_regs.status.n = output & 0x80;
    m_regs.status.z = output == 0x00;

    return output & 0xFF;
  }

  case RTI: {
    m_regs.status.bits = stack_pop();
    m_regs.status.b = 0;
    m_regs.status._ = 1;

    m_regs.pc = ((stack_pop()) | (stack_pop() << 8));
  } break;

  case RTS: {
    m_regs.pc = ((stack_pop()) | (stack_pop() << 8)) + 1;
  } break;

  case SBC: {
    m_regs.a = add_with_carry(~operand);
  } break;

  case SEC: {
    m_regs.status.c = 1;
  } break;

  case SED: {
    m_regs.status.d = 1;
  } break;

  case SEI: {
    m_regs.status.i = 1;
  } break;

  case STA: {
    return m_regs.a;
  }

  case STX: {
    return m_regs.x;
  }

  case STY: {
    return m_regs.y;
  }

  case TAX: {
    m_regs.x = parse_operand(m_regs.a);
  } break;

  case TAY: {
    m_regs.y = parse_operand(m_regs.a);
  } break;

  case TSX: {
    m_regs.x = parse_operand(m_regs.sp);
  } break;

  case TXA: {
    m_regs.a = parse_operand(m_regs.x);
  } break;

  case TXS: {
    m_regs.sp = m_regs.x;
  } break;

  case TYA: {
    m_regs.a = parse_operand(m_regs.y);
  } break;

  case ILL: {
    // throw Exception {"CPU does not implement ILL opcodes (ALR, TAS, SHX, ...)"};
  } break;

  default: break;
  }

  return std::nullopt;
}

uint8 Cpu::parse_operand(uint8 operand) {
  m_regs.status.z = operand == 0x00;
  m_regs.status.n = operand & 0x80;

  return operand;
}

uint8 Cpu::add_with_carry(uint8 operand) {
  uint16 sum = m_regs.a + operand + m_regs.status.c;
  uint8 output = sum & 0xFF;

  m_regs.status.c = sum > 0xFF;
  m_regs.status.z = output == 0x00;
  m_regs.status.v = (m_regs.a ^ sum) & (operand ^ sum) & 0x80;
  m_regs.status.n = output & 0x80;

  return output;
}

uint8 Cpu::bitwise_fn(uint8 operand, uint8 (*fn)(uint8 a, uint8 b)) {
  uint8 output = (*fn)(m_regs.a, operand);

  m_regs.status.z = output == 0x00;
  m_regs.status.n = output & 0x80;

  return output;
}

void Cpu::branch(bool condition, uint8 offset) {
  uint16 destination = m_regs.pc + static_cast<int8>(offset);

  if (condition) {
    if ((m_regs.pc & 0xFF00) != (destination & 0xFF00)) {
      m_cycles_remaining += 2;
    } else {
      m_cycles_remaining += 1;
    }

    m_regs.pc = destination;
  }
}

void Cpu::compare(uint8 a, uint8 b) {
  m_regs.status.c = a >= b;
  m_regs.status.z = a == b;
  m_regs.status.n = (a - b) & 0x80;
}

uint8 Cpu::stack_push(uint8 data) {
  return m_bus.cpu_write(0x0100 + m_regs.sp--, data);
}

uint8 Cpu::stack_pop() {
  return m_bus.cpu_read(0x0100 + ++m_regs.sp);
}

}  // namespace nemu
