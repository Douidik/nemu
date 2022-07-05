#include "cpu.hpp"
#include "bus.hpp"
#include "exception.hpp"

namespace nemu {

using enum Mode;
using enum Mnemonic;

Cpu::Cpu(Bus *bus) : Hardware {bus} {}

void Cpu::init() {
  m_bus.ram() = {}, m_cycles_remaining = 0, m_instruction_counter = 0, m_disasm = {};

  m_regs = CpuRegisters {
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
  if (!m_cycles_remaining--) {
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

  m_regs.status.bits |= interrupt.status_mask;

  return m_bus.cpu_read(interrupt.vector + 1) << 8 | m_bus.cpu_read(interrupt.vector);
}

void Cpu::parse_instruction(Instruction instruction) {
  m_disasm = Disasm {instruction, {}};

  switch (instruction.mode) {
  case ACC: {
    auto output = execute_operation(instruction, m_disasm.operand = m_regs.a);

    if (output.has_value()) {
      m_regs.a = output.value();
    }
  } break;

  case IMM: {
    execute_operation(instruction, m_disasm.operand = m_bus.cpu_read(m_regs.pc++));
  } break;

  case IMP: {
    execute_operation(instruction, {});
  } break;

  case REL: {
    uint16 offset = m_bus.cpu_read(m_regs.pc++);

    if (offset & 0x80) {
      offset |= 0xFF00;
    }

    execute_operation(instruction, offset);
  } break;

  case ABS:
  case ABX:
  case ABY:
  case IND:
  case IDX:
  case IDY:
  case ZER:
  case ZPX:
  case ZPY: {
    uint16 address = m_disasm.operand = read_operand_address(instruction);

    switch (instruction.mnemonic) {
    case JMP:
    case JSR: {  // Jump instructions
      execute_operation(instruction, address);
    } break;

    case STA:
    case STX:
    case STY: {  // Write instructions
      m_bus.cpu_write(address, *execute_operation(instruction, {}));
    } break;

    default: {  // R/W instructions
      auto output = execute_operation(instruction, m_bus.cpu_read(address));

      if (output.has_value()) {
        m_bus.cpu_write(address, *output);
      }
    } break;
    }
  } break;
  }

  m_cycles_remaining += instruction.cycles;
}

uint16 Cpu::read_operand_address(Instruction instruction) {
  const auto absolute = [&](uint8 offset) -> uint16 {
    uint8 address_bytes[] = {
      m_bus.cpu_read(m_regs.pc++),
      m_bus.cpu_read(m_regs.pc++),
    };

    return (address_bytes[1] << 8 | address_bytes[0]) + offset;
  };

  const auto zero_page = [&](uint8 offset) -> uint16 {
    return (m_bus.cpu_read(m_regs.pc++) + offset) & 0xFF;
  };

  switch (instruction.mode) {
  case ABS: return absolute(0);
  case ABX: return absolute(m_regs.x);
  case ABY: return absolute(m_regs.y);

  case ZER: return zero_page(0);
  case ZPX: return zero_page(m_regs.x);
  case ZPY: return zero_page(m_regs.y);

  case IND: {
    uint8 pointer_bytes[] = {
      m_bus.cpu_read(m_regs.pc++),
      m_bus.cpu_read(m_regs.pc++),
    };

    uint16 pointer = (pointer_bytes[1] << 8) | pointer_bytes[0];

    uint8 address_bytes[] = {
      m_bus.cpu_read(pointer),
      // 6502 page boundary bug emulation
      pointer_bytes[0] != 0xFF ? m_bus.cpu_read(pointer & 0xFF00) : m_bus.cpu_read(pointer + 1),
    };

    return address_bytes[1] << 8 | address_bytes[0];
  }

  case IDX: {
    uint8 base = m_bus.cpu_read(m_regs.pc++) + m_regs.x;

    uint8 address_bytes[] = {
      m_bus.cpu_read(base++ & 0xFF),
      m_bus.cpu_read(base++ & 0xFF),
    };

    return address_bytes[1] << 8 | address_bytes[0];
  }

  case IDY: {
    uint8 base = m_bus.cpu_read(m_regs.pc++);

    uint8 base_bytes[] = {
      m_bus.cpu_read(base++ & 0xFF),
      m_bus.cpu_read(base++ & 0xFF),
    };

    uint16 address = (base_bytes[1] << 8 | base_bytes[0]) + m_regs.y;

    if ((address & 0xFF00) != base_bytes[1] << 8) {
      m_cycles_remaining++;
    }

    return address;
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
    stack_push(m_regs.status.bits | (0b00110000));
  } break;

  case PLA: {
    m_regs.a = parse_operand(stack_pop());
  } break;

  case PLP: {
    // TODO: fix
    // m_regs.status.bits = stack_pop() | (m_regs.status.bits & 0b00110000);
    auto temp = m_regs.status;
    m_regs.status.bits = stack_pop();
    m_regs.status.b = temp.b, m_regs.status._ = temp._;
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
    m_cycles_remaining++;
    // throw Exception {"CPU does not implement ILL opcodes"};
  }

  case NOP: break;
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
  if (condition) {
    uint16 destination = m_regs.pc + static_cast<int8>(offset);

    if ((m_regs.pc & 0xFF00) != (destination & 0xFF00)) {
      m_cycles_remaining += 2;
    } else {
      m_cycles_remaining += 1;
    }

    m_regs.pc = m_disasm.operand = destination;
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
