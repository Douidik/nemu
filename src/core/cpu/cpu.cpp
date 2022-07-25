#include "cpu.hpp"
#include "bus.hpp"
#include "exception.hpp"
#include <optional>

namespace nemu {

using namespace mnemonics;
using namespace modes;

Cpu::Cpu(Bus *bus) : Hardware {bus} {}

void Cpu::init() {
  m_bus.ram() = {}, m_cycles_remaining = 0, m_instruction_counter = 0;

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
  m_cycles_remaining += interrupt.cycles;
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
    uint16 address = read_address(instruction);

    if (instruction.mnemonic & JUMP) {
      execute_operation(instruction, address);
    }

    if (instruction.mnemonic & STORE) {
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

uint16 Cpu::read_address(Instruction instruction) {
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
      pointer_bytes[0] != 0xFF ? m_bus.cpu_read(pointer + 1) : m_bus.cpu_read(pointer & 0xFF00),
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
    stack_push(m_regs.status.bits | (0b0011'0000));
  } break;

  case PLA: {
    m_regs.a = parse_operand(stack_pop());
  } break;

  case PLP: {
    // TODO: fix
    // m_regs.status.bits = stack_pop() | (m_regs.status.bits & 0b0011'0000);
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

Disasm Cpu::disasm() const {
  auto opcode = m_bus.cpu_peek(m_regs.pc);

  if (!opcode) {
    return {};  // Not much to do ...
  }

  auto instruction = INSTRUCTION_SET[*opcode];
  auto address = disasm_fetch(instruction);
  auto operation = disasm_operation(instruction, address);
  auto bytes = disasm_bytes(instruction);

  return Disasm {instruction, operation, address, bytes, m_regs};
}

Disasm::Bytes Cpu::disasm_bytes(Instruction instruction) const {
  Disasm::Bytes bytes(instruction.size());

  for (uint16 n = 0; n < bytes.size(); n++) {
    bytes[n] = m_bus.cpu_peek(m_regs.pc + n).value_or(0);
  }

  return bytes;
}

Disasm::Fetch Cpu::disasm_fetch(Instruction instruction) const {
  // Read the next 2 bytes that may contain our address
  uint8 bytes[] {
    m_bus.cpu_peek(m_regs.pc + 1).value_or(0),
    m_bus.cpu_peek(m_regs.pc + 2).value_or(0),
  };

  // Make a 16-bit address from the 2 bytes we read
  uint16 address = bytes[1] << 8 | bytes[0];

  switch (instruction.mode) {
  case ACC: {
    return {.acc = {m_regs.a}};
  }

  case ABS: {
    return {.abs = {address, *m_bus.cpu_peek(address)}};
  }

  case ABX: {
    uint16 destination = address + m_regs.x;

    return {
      .abx = {destination, address, m_regs.x, *m_bus.cpu_peek(destination)},
    };
  }

  case ABY: {
    uint16 destination = address + m_regs.y;

    return {
      .aby = {destination, address, m_regs.y, *m_bus.cpu_peek(destination)},
    };
  }

  case IMM: {
    return {.imm = bytes[0]};
  }

  case IMP: {
    return {.imp {}};
  }

  case IND: {
    uint16 pointer = (bytes[1] << 8) | bytes[0];

    uint8 destination_bytes[] = {
      *m_bus.cpu_peek(pointer),
      // 6502 page boundary bug emulation
      bytes[0] != 0xFF ? *m_bus.cpu_peek(pointer + 1) : *m_bus.cpu_peek(pointer & 0xFF00),
    };

    uint16 destination = destination_bytes[1] << 8 | destination_bytes[0];

    return {
      .ind = {destination, address, *m_bus.cpu_peek(destination)},
    };
  }

  case IDX: {
    uint8 base = bytes[0] + m_regs.x;

    uint8 address_bytes[] = {
      *m_bus.cpu_peek(base++ & 0xFF),
      *m_bus.cpu_peek(base++ & 0xFF),
    };

    uint16 destination = address_bytes[1] << 8 | address_bytes[0];

    return {
      .idx = {destination, bytes[0], m_regs.x, *m_bus.cpu_peek(destination)},
    };
  }

  case IDY: {
    uint8 base = bytes[0];

    uint8 base_bytes[] = {
      *m_bus.cpu_peek(base++ & 0xFF),
      *m_bus.cpu_peek(base++ & 0xFF),
    };

    uint16 destination = (base_bytes[1] << 8 | base_bytes[0]) + m_regs.y;

    return {
      .idy = {destination, bytes[0], m_regs.y, *m_bus.cpu_peek(destination)},
    };
  }

  case REL: {
    int8 offset = static_cast<int8>(bytes[0]) + instruction.size();
    uint16 destination = m_regs.pc + offset;

    return {
      .rel {destination, m_regs.pc, offset},
    };
  }

  case ZER: {
    return {.zer = {bytes[0], *m_bus.cpu_peek(bytes[0])}};
  }

  case ZPX: {
    uint8 destination = (bytes[0] + m_regs.x) & 0xFF;

    return {
      .zpx = {destination, bytes[0], m_regs.x, *m_bus.cpu_peek(destination)},
    };
  }

  case ZPY: {
    uint8 destination = (bytes[0] + m_regs.y) & 0xFF;

    return {
      .zpy = {destination, bytes[0], m_regs.y, *m_bus.cpu_peek(destination)},
    };
  }

  default: return {};
  }
}

Disasm::Operation Cpu::disasm_operation(Instruction instruction, Disasm::Fetch fetch) const {
  constexpr auto make_description = [](Instruction instruction) -> std::string_view {
    switch (instruction.mnemonic) {
    case LDA: return "a = {data :02X}";
    case LDX: return "x = {data :02X}";
    case LDY: return "y = {data :02X}";

    case PHA: return "s[{index :02X}] = a";
    case PHP: return "s[{index :02X}] = status";
    case PLA: return "a = s[{index :02X}]";
    case PLP: return "status = s[{index :02X}]";

    case DEC:
    case DEX:
    case DEY: return "{data :02X}--";
    case INC:
    case INX:
    case INY: return "{data :02X}++";

    case ADC: return "{a :02X} + {b :02X}";
    case SBC: return "{a :02X} - {b :02X}";
    case AND: return "{a :02X} & {b :02X}";
    case EOR: return "{a :02X} ^ {b :02X}";
    case ORA: return "{a :02X} | {b :02X}";

    case ASL: return "C << [{data :08b}] << 0";
    case LSR: return "0 >> [{data :08b}] >> C";
    case ROL: return "C << [{data :08b}] << C";
    case ROR: return "C >> [{data :08b}] >> C";

    case CLC: return "C = 0";
    case CLD: return "D = 0";
    case CLI: return "I = 0";
    case CLV: return "V = 0";
    case SEC: return "C = 1";
    case SED: return "D = 1";
    case SEI: return "I = 1";

    default: return "?";
    }
  };

  auto make_conditional = [this](Instruction instruction) -> Disasm::Conditional {
    switch (instruction.mnemonic) {
    case BCC: return {"C", m_regs.status.c};
    case BCS: return {"!C", m_regs.status.c};
    case BEQ: return {"Z", m_regs.status.z};
    case BMI: return {"N", m_regs.status.n};
    case BNE: return {"!Z", m_regs.status.z};
    case BPL: return {"!N", m_regs.status.n};
    case BVC: return {"!V", m_regs.status.v};
    case BVS: return {"V", m_regs.status.v};

    default: return {};
    }
  };

  auto make_transfer = [this](Instruction instruction) -> Disasm::Transfer {
    switch (instruction.mnemonic) {
    case TAX: return {{"a", m_regs.a}, {"x", m_regs.x}};
    case TAY: return {{"a", m_regs.a}, {"y", m_regs.y}};
    case TSX: return {{"stack_pointer", m_regs.sp}, {"x", m_regs.x}};
    case TXA: return {{"x", m_regs.x}, {"a", m_regs.a}};
    case TXS: return {{"x", m_regs.x}, {"stack_pointer", m_regs.sp}};
    case TYA: return {{"y", m_regs.y}, {"a", m_regs.a}};

    default: return {};
    }
  };

  if (instruction.mnemonic & LOAD) {
    return Disasm::Load {};
  }

  if (instruction.mnemonic & TRANSFER) {
    return Disasm::Transfer {make_transfer(instruction)};
  }

  if (instruction.mnemonic & STACK) {
    uint8 stack_pointer {};

    if (instruction.mnemonic & (PHA | PHP)) {
      stack_pointer = m_regs.sp + 1;
    }

    if (instruction.mnemonic & (PLA | PLP)) {
      stack_pointer = m_regs.sp;
    }

    return Disasm::Stack {make_description(instruction), stack_pointer};
  }

  if (instruction.mnemonic & INCREMENT) {
    uint8 data {};

    if (instruction.mnemonic & (INC | DEC)) {
      data = *fetched_data;
    }

    if (instruction.mnemonic & (INX | DEX)) {
      data = m_regs.x;
    }

    if (instruction.mnemonic & (INY | DEY)) {
      data = m_regs.y;
    }

    return Disasm::Increment {make_description(instruction), data};
  }

  if (instruction.mnemonic & BINARY_OP) {
    return Disasm::BinaryOp {make_description(instruction), m_regs.a, *fetched_data};
  }

  if (instruction.mnemonic & SHIFT) {
    return Disasm::Shift {make_description(instruction), *fetched_data};
  }

  if (instruction.mnemonic & FLAG) {
    return Disasm::Flag {make_description(instruction)};
  }

  if (instruction.mnemonic & COMPARISON) {
    return Disasm::Comparison {m_regs.a, *fetched_data};
  }

  if (instruction.mnemonic & CONDITIONAL) {
    return Disasm::Conditional {make_conditional(instruction)};
  }

  if (instruction.mnemonic & JUMP) {
    return Disasm::Jump {};
  }

  if (instruction.mnemonic & INTERRUPT) {
    return Disasm::Interrupt {};
  }

  if (instruction.mnemonic & OTHER) {
    return Disasm::OtherOp {};
  }

  return {};
}

}  // namespace nemu
