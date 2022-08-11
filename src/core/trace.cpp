#include "trace.hpp"

namespace nemu {
using namespace cpu;

Trace::Trace(const Bus *bus) : m_bus {*bus} {}

Disasm Trace::disasm() const {
  uint8 opcode {
    m_bus.cpu_peek(m_bus.cpu().program_counter()),
  };

  const auto &instruction = INSTRUCTION_SET[opcode];

  Disasm disasm {
    m_bus.cpu().registers(),
    disasm_bytes(disasm, instruction.size()),
    disasm_fetch(disasm, instruction.mode),
    disasm_operation(disasm, instruction.mnemonic),
  };

  return disasm;
}

Disasm::Bytes Trace::disasm_bytes(const Disasm &disasm, size_t size) const {
  Disasm::Bytes bytes(size);

  for (uint16 n = 0; n < bytes.size(); n++) {
    bytes[n] = m_bus.cpu_peek(m_bus.cpu().program_counter() + n);
  }

  return bytes;
}

Disasm::Fetch Trace::disasm_fetch(const Disasm &disasm, Mode mode) const {
  const auto &registers = m_bus.cpu().registers();
  const auto &bytes = disasm.bytes;

  auto disasm_absolute = [&](std::string_view description, uint8 offset) -> Disasm::Fetch {
    uint16 address = bytes[2] << 8 | bytes[1];
    uint16 destination = address + offset;

    return {mode, description, address, destination, offset, m_bus.cpu_peek(destination)};
  };

  auto disasm_zero_page = [&](std::string_view description, uint8 offset) -> Disasm::Fetch {
    uint16 address = bytes[1];
    uint16 destination = (address + offset) & 0xFF;

    return {mode, description, address, destination, offset, m_bus.cpu_peek(destination)};
  };

  switch (mode) {
  case ACC: {
    return {ACC, "[=: #${:02X}]", {}, {}, {}, registers.a};
  }

  case ABS: {
    return disasm_absolute("${:04X} [=: #${:02X}]", {});
  }

  case ABX: {
    return disasm_absolute("${:04X},x [=: #${:02X}, x: #${:02X}, dst: ${:04X}]", registers.x);
  }

  case ABY: {
    return disasm_absolute("${:04X},x [=: #${:02X}, y: #${:02X}, dst: ${:04X}]", registers.y);
  }

  case IMM: {
    return {IMM, "#${:02X}", {}, {}, {}, bytes[1]};
  }

  case IMP: {
    return {IMP};
  }

  case IND: {
    uint16 address = bytes[2] << 8 | bytes[1];

    uint8 destination_bytes[] = {
      m_bus.cpu_peek(address),
      // 6502 page boundary bug emulation
      bytes[0] != 0xFF ? m_bus.cpu_peek(address + 1) : m_bus.cpu_peek(address & 0xFF00),
    };

    uint16 destination = destination_bytes[1] << 8 | destination_bytes[0];

    return {
      IND,
      "(${:04X}) [=: #${:02X}, dst: ${:04X}]",
      address,
      destination,
      {},
      m_bus.cpu_peek(destination),
    };
  }

  case IDX: {
    uint16 address = bytes[1];

    uint8 destination_bytes[] = {
      m_bus.cpu_peek((address + registers.x + 0) & 0xFF),
      m_bus.cpu_peek((address + registers.x + 1) & 0xFF),
    };

    uint16 destination = destination_bytes[1] << 8 | destination_bytes[0];

    return {
      IDX,
      "(${:02},x) [=: #${:02X}, x: #${:02X}, dst: ${:04X}]",
      address,
      destination,
      registers.x,
      m_bus.cpu_peek(destination),
    };
  }

  case IDY: {
    uint16 address = bytes[1];

    uint8 destination_bytes[] = {
      m_bus.cpu_peek((address + 0) & 0xFF),
      m_bus.cpu_peek((address + 1) & 0xFF),
    };

    uint16 destination = (destination_bytes[1] << 8 | destination_bytes[0]) + registers.y;

    return {
      IDY,
      "(${:02X}),y [=: #${:02X}, y: #${:02X}, dst: ${:04X}]",
      address,
      destination,
      registers.y,
      m_bus.cpu_peek(destination),
    };
  }

  case REL: {
    int8 offset = static_cast<int16>(bytes[1]) + bytes.size();
    uint16 destination = m_bus.cpu().program_counter() + offset;

    return {REL, "${:04X}", {}, destination, static_cast<uint8>(offset), {}};
  }

  case ZER: {
    return disasm_zero_page("${:02X} [=: #${:02X}]", {});
  }

  case ZPX: {
    return disasm_zero_page("${:02X},x [=: #${:02X}, x: #${:02X}, dst: ${:04X}]", registers.x);
  }

  case ZPY: {
    return disasm_zero_page("${:02X},y [=: #${:02X}, y: #${:02X}, dst: ${:04X}]", registers.y);
  }

  default: return {};
  }
}

Disasm::Operation Trace::disasm_operation(const Disasm &disasm, Mnemonic mnemonic) const {
  constexpr auto make_description = [](Mnemonic mnemonic) -> std::string_view {
    switch (mnemonic) {
    case CMP:
    case CPX:
    case CPY: return "{:02X} <=> {:02X} ({}, {}, {})";

    case LDA: return "a = {:02X}";
    case LDX: return "x = {:02X}";
    case LDY: return "y = {:02X}";

    case PHA: return "s[{:02X}] = a";
    case PHP: return "s[{:02X}] = status";
    case PLA: return "a = s[{:02X}]";
    case PLP: return "status = s[{:02X}]";

    case DEC:
    case DEX:
    case DEY: return "{:02X} -= 1";
    case INC:
    case INX:
    case INY: return "{:02X} += 1";

    case ADC: return "{:02X} + {:02X}";
    case SBC: return "{:02X} - {:02X}";
    case AND: return "{:02X} & {:02X}";
    case EOR: return "{:02X} ^ {:02X}";
    case ORA: return "{:02X} | {:02X}";

    case ASL: return "C << [{:08b}] << 0";
    case LSR: return "0 >> [{:08b}] >> C";
    case ROL: return "C << [{:08b}] << C";
    case ROR: return "C >> [{:08b}] >> C";

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

  constexpr auto make_conditional =
    [](Mnemonic mnemonic, const Registers &registers) -> Disasm::Operation {
    switch (mnemonic) {
    case BCC: return {BCC, "C ? {}", {registers.status.c}};
    case BEQ: return {BEQ, "Z ? {}", {registers.status.z}};
    case BMI: return {BMI, "N ? {}", {registers.status.n}};
    case BVS: return {BVS, "V ? {}", {registers.status.v}};

    case BCS: return {BCS, "!C ? {}", {registers.status.c}};
    case BNE: return {BNE, "!Z ? {}", {registers.status.z}};
    case BPL: return {BPL, "!N ? {}", {registers.status.n}};
    case BVC: return {BVC, "!V ? {}", {registers.status.v}};

    default: return {mnemonic, "?"};
    }
  };

  constexpr auto make_transfer =
    [](Mnemonic mnemonic, const Registers &registers) -> Disasm::Operation {
    switch (mnemonic) {
    case TAX: return {TAX, "a: #${:02X} => x: #${:02X}", {registers.a, registers.x}};
    case TAY: return {TAY, "a: #${:02X} => y: #${:02X}", {registers.a, registers.y}};
    case TXA: return {TXA, "x: #${:02X} => a: #${:02X}", {registers.x, registers.a}};
    case TYA: return {TYA, "y: #${:02X} => a: #${:02X}", {registers.y, registers.a}};
    case TXS: return {TXS, "x: #${:02X} => sp: #${:02X}", {registers.x, registers.sp}};
    case TSX: return {TSX, "sp: #${:02X} => x: #${:02X}", {registers.sp, registers.x}};

    default: return {mnemonic, "?"};
    }
  };

  const auto &registers = m_bus.cpu().registers();
  const auto &fetch = disasm.fetch;

  if (mnemonic & TRANSFER) {
    return make_transfer(mnemonic, registers);
  }

  if (mnemonic & STACK) {
    uint8 sp {};

    if (mnemonic & (PHA | PHP)) {
      sp = registers.sp;
    }

    if (mnemonic & (PLA | PLP)) {
      sp = registers.sp;
    }

    return {mnemonic, make_description(mnemonic), {sp}};
  }

  if (mnemonic & INCREMENT) {
    uint8 data {};

    if (mnemonic & (INC | DEC)) {
      data = fetch.data;
    }

    if (mnemonic & (INX | DEX)) {
      data = registers.x;
    }

    if (mnemonic & (INY | DEY)) {
      data = registers.y;
    }

    return {mnemonic, make_description(mnemonic), {data}};
  }

  if (mnemonic & BINARY_OP) {
    return {mnemonic, make_description(mnemonic), {registers.a, fetch.data}};
  }

  if (mnemonic & SHIFT) {
    return {mnemonic, make_description(mnemonic), {fetch.data}};
  }

  if (mnemonic & FLAG) {
    return {mnemonic, make_description(mnemonic)};
  }

  if (mnemonic & COMPARISON) {
    uint8 operand {};

    if (mnemonic & CMP) {
      operand = registers.a;
    }

    if (mnemonic & CPX) {
      operand = registers.x;
    }

    if (mnemonic & CPY) {
      operand = registers.y;
    }

    return {mnemonic, make_description(mnemonic), {operand, fetch.data}};
  }

  if (mnemonic & CONDITIONAL) {
    return make_conditional(mnemonic, registers);
  }

  return Disasm::Operation {mnemonic};
}

}  // namespace nemu
