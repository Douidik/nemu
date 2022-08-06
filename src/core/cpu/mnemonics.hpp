#ifndef NEMU_CPU_MNEMONICS_HPP
#define NEMU_CPU_MNEMONICS_HPP

#include <fmt/core.h>

namespace nemu::mnemonics {

enum Mnemonic {
  ILL = 1UL << 0x00,
  ADC = 1UL << 0x01,
  AND = 1UL << 0x02,
  ASL = 1UL << 0x03,
  BCC = 1UL << 0x04,
  BCS = 1UL << 0x05,
  BEQ = 1UL << 0x06,
  BIT = 1UL << 0x07,
  BMI = 1UL << 0x08,
  BNE = 1UL << 0x09,
  BPL = 1UL << 0x0A,
  BRK = 1UL << 0x0B,
  BVC = 1UL << 0x0C,
  BVS = 1UL << 0x0D,
  CLC = 1UL << 0x0E,
  CLD = 1UL << 0x0F,
  CLI = 1UL << 0x10,
  CLV = 1UL << 0x11,
  CMP = 1UL << 0x12,
  CPX = 1UL << 0x13,
  CPY = 1UL << 0x14,
  DEC = 1UL << 0x15,
  DEX = 1UL << 0x16,
  DEY = 1UL << 0x17,
  EOR = 1UL << 0x18,
  INC = 1UL << 0x19,
  INX = 1UL << 0x1A,
  INY = 1UL << 0x1B,
  JMP = 1UL << 0x1C,
  JSR = 1UL << 0x1D,
  LDA = 1UL << 0x1E,
  LDX = 1UL << 0x1F,
  LDY = 1UL << 0x20,
  LSR = 1UL << 0x21,
  NOP = 1UL << 0x22,
  ORA = 1UL << 0x23,
  PHA = 1UL << 0x24,
  PHP = 1UL << 0x25,
  PLA = 1UL << 0x26,
  PLP = 1UL << 0x27,
  ROL = 1UL << 0x28,
  ROR = 1UL << 0x29,
  RTI = 1UL << 0x2A,
  RTS = 1UL << 0x2B,
  SBC = 1UL << 0x2C,
  SEC = 1UL << 0x2D,
  SED = 1UL << 0x2E,
  SEI = 1UL << 0x2F,
  STA = 1UL << 0x30,
  STX = 1UL << 0x31,
  STY = 1UL << 0x32,
  TAX = 1UL << 0x33,
  TAY = 1UL << 0x34,
  TSX = 1UL << 0x35,
  TXA = 1UL << 0x36,
  TXS = 1UL << 0x37,
  TYA = 1UL << 0x38,

  // INSTRUCTION CATEGORIES

  LOAD = LDA | LDX | LDY,
  STORE = STA | STX | STY,
  TRANSFER = TAX | TAY | TSX | TXA | TXS | TYA,
  STACK = PHA | PHP | PLA | PLP,
  INCREMENT = DEC | DEX | DEY | INC | INX | INY,
  BINARY_OP = ADC | SBC | AND | EOR | ORA,
  SHIFT = ASL | LSR | ROL | ROR,
  FLAG = CLC | CLD | CLI | CLV | SEC | SED | SEI,
  COMPARISON = CMP | CPX | CPY,
  CONDITIONAL = BCC | BCS | BEQ | BMI | BNE | BPL | BVC | BVS,
  JUMP = JMP | JSR | RTS,
  INTERRUPT = BRK | RTI,
  OTHER = ILL | BIT | NOP,
};

}  // namespace nemu::mnemonics

namespace fmt {
using namespace nemu::mnemonics;

template<>
struct formatter<Mnemonic> {
  constexpr std::string_view name(Mnemonic mnemonic) const {
    switch (mnemonic) {
    case ILL: return "ILL";
    case ADC: return "ADC";
    case AND: return "AND";
    case ASL: return "ASL";
    case BCC: return "BCC";
    case BCS: return "BCS";
    case BEQ: return "BEQ";
    case BIT: return "BIT";
    case BMI: return "BMI";
    case BNE: return "BNE";
    case BPL: return "BPL";
    case BRK: return "BRK";
    case BVC: return "BVC";
    case BVS: return "BVS";
    case CLC: return "CLC";
    case CLD: return "CLD";
    case CLI: return "CLI";
    case CLV: return "CLV";
    case CMP: return "CMP";
    case CPX: return "CPX";
    case CPY: return "CPY";
    case DEC: return "DEC";
    case DEX: return "DEX";
    case DEY: return "DEY";
    case EOR: return "EOR";
    case INC: return "INC";
    case INX: return "INX";
    case INY: return "INY";
    case JMP: return "JMP";
    case JSR: return "JSR";
    case LDA: return "LDA";
    case LDX: return "LDX";
    case LDY: return "LDY";
    case LSR: return "LSR";
    case NOP: return "NOP";
    case ORA: return "ORA";
    case PHA: return "PHA";
    case PHP: return "PHP";
    case PLA: return "PLA";
    case PLP: return "PLP";
    case ROL: return "ROL";
    case ROR: return "ROR";
    case RTI: return "RTI";
    case RTS: return "RTS";
    case SBC: return "SBC";
    case SEC: return "SEC";
    case SED: return "SED";
    case SEI: return "SEI";
    case STA: return "STA";
    case STX: return "STX";
    case STY: return "STY";
    case TAX: return "TAX";
    case TAY: return "TAY";
    case TSX: return "TSX";
    case TXA: return "TXA";
    case TXS: return "TXS";
    case TYA: return "TYA";

    default: return "?";
    }
  }

  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template<typename F>
  constexpr auto format(const Mnemonic &mnemonic, F &context) const {
    std::string_view mn = name(mnemonic);
    return format_to(context.out(), "{}", name(mnemonic));
  }
};
  
}  // namespace fmt

#endif
