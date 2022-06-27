#ifndef NEMU_CPU_INSTRUCTIONS_HPP
#define NEMU_CPU_INSTRUCTIONS_HPP

#include "int.hpp"
#include "mnemonics.hpp"
#include "modes.hpp"

namespace nemu {

struct Instruction {
  Mnemonic mnemonic {Mnemonic::ILL};
  Mode mode {Mode::IMP};
  uint8 cycles {};
};

namespace {
  using enum Mode;
  using enum Mnemonic;

  constexpr inline Instruction INSTRUCTION_SET[256] = {
    [0x69] = {ADC, IMM, 2}, [0x65] = {ADC, ZER, 3}, [0x75] = {ADC, ZPX, 4}, [0x6D] = {ADC, ABS, 4},
    [0x7D] = {ADC, ABX, 4}, [0x79] = {ADC, ABY, 4}, [0x61] = {ADC, IDX, 6}, [0x71] = {ADC, IDY, 5},

    [0x29] = {AND, IMM, 2}, [0x25] = {AND, ZER, 3}, [0x35] = {AND, ZPX, 4}, [0x2D] = {AND, ABS, 4},
    [0x3D] = {AND, ABX, 4}, [0x39] = {AND, ABY, 4}, [0x21] = {AND, IDX, 6}, [0x31] = {AND, IDY, 5},

    [0x0A] = {ASL, ACC, 2}, [0x06] = {ASL, ZER, 5}, [0x16] = {ASL, ZPX, 6}, [0x0E] = {ASL, ABS, 6},
    [0x1E] = {ASL, ABX, 7},

    [0x90] = {BCC, REL, 2}, [0xB0] = {BCS, REL, 2}, [0xF0] = {BEQ, REL, 2}, [0x30] = {BMI, REL, 2},
    [0xD0] = {BNE, REL, 2}, [0x10] = {BPL, REL, 2}, [0x50] = {BVC, REL, 2}, [0x70] = {BVS, REL, 2},

    [0x24] = {BIT, ZER, 3}, [0x2C] = {BIT, ABS, 4},

    [0x00] = {BRK, IMP, 7},

    [0x18] = {CLC, IMP, 2}, [0xD8] = {CLD, IMP, 2}, [0x58] = {CLI, IMP, 2}, [0xB8] = {CLV, IMP, 2},

    [0xC9] = {CMP, IMM, 2}, [0xC5] = {CMP, ZER, 3}, [0xD5] = {CMP, ZPX, 4}, [0xCD] = {CMP, ABS, 4},
    [0xDD] = {CMP, ABX, 4}, [0xD9] = {CMP, ABY, 4}, [0xC1] = {CMP, IDX, 6}, [0xD1] = {CMP, IDY, 5},

    [0xE0] = {CPX, IMM, 2}, [0xE4] = {CPX, ZER, 3}, [0xEC] = {CPX, ABS, 4},

    [0xC0] = {CPY, IMM, 2}, [0xC4] = {CPY, ZER, 3}, [0xCC] = {CPY, ABS, 4},

    [0xC6] = {DEC, ZER, 5}, [0xD6] = {DEC, ZPX, 6}, [0xCE] = {DEC, ABS, 6}, [0xDE] = {DEC, ABX, 7},

    [0xCA] = {DEX, IMP, 2}, [0x88] = {DEY, IMP, 2},

    [0x49] = {EOR, IMM, 2}, [0x45] = {EOR, ZER, 3}, [0x55] = {EOR, ZPX, 4}, [0x4D] = {EOR, ABS, 4},
    [0x5D] = {EOR, ABX, 4}, [0x59] = {EOR, ABY, 4}, [0x41] = {EOR, IDX, 6}, [0x51] = {EOR, IDY, 5},

    [0xE6] = {INC, ZER, 5}, [0xF6] = {INC, ZPX, 6}, [0xEE] = {INC, ABS, 6}, [0xFE] = {INC, ABX, 7},

    [0xE8] = {INX, IMP, 2}, [0xC8] = {INY, IMP, 2},

    [0x4C] = {JMP, ABS, 3}, [0x6C] = {JMP, IND, 5},

    [0x20] = {JSR, ABS, 6},

    [0xA9] = {LDA, IMM, 2}, [0xA5] = {LDA, ZER, 3}, [0xB5] = {LDA, ZPX, 4}, [0xAD] = {LDA, ABS, 4},
    [0xBD] = {LDA, ABX, 4}, [0xB9] = {LDA, ABY, 4}, [0xA1] = {LDA, IDX, 6}, [0xB1] = {LDA, IDY, 5},

    [0xA2] = {LDX, IMM, 2}, [0xA6] = {LDX, ZER, 3}, [0xB6] = {LDX, ZPY, 4}, [0xAE] = {LDX, ABS, 4},
    [0xBE] = {LDX, ABY, 4},

    [0xA0] = {LDY, IMM, 2}, [0xA4] = {LDY, ZER, 3}, [0xB4] = {LDY, ZPX, 4}, [0xAC] = {LDY, ABS, 4},
    [0xBC] = {LDY, ABX, 4},

    [0x4A] = {LSR, ACC, 2}, [0x46] = {LSR, ZER, 5}, [0x56] = {LSR, ZPX, 6}, [0x4E] = {LSR, ABS, 6},
    [0x5E] = {LSR, ABX, 7},

    [0xEA] = {NOP, IMP, 2},

    [0x09] = {ORA, IMM, 2}, [0x05] = {ORA, ZER, 3}, [0x15] = {ORA, ZPX, 4}, [0x0D] = {ORA, ABS, 4},
    [0x1D] = {ORA, ABX, 4}, [0x19] = {ORA, ABY, 4}, [0x01] = {ORA, IDX, 6}, [0x11] = {ORA, IDY, 5},

    [0x48] = {PHA, IMP, 3}, [0x08] = {PHP, IMP, 3},

    [0x68] = {PLA, IMP, 4}, [0x28] = {PLP, IMP, 4},

    [0x2A] = {ROL, ACC, 2}, [0x26] = {ROL, ZER, 5}, [0x36] = {ROL, ZPX, 6}, [0x2E] = {ROL, ABS, 6},
    [0x3E] = {ROL, ABX, 7},

    [0x6A] = {ROR, ACC, 2}, [0x66] = {ROR, ZER, 5}, [0x76] = {ROR, ZPX, 6}, [0x6E] = {ROR, ABS, 6},
    [0x7E] = {ROR, ABX, 7},

    [0x40] = {RTI, IMP, 6}, [0x60] = {RTS, IMP, 6},

    [0xE9] = {SBC, IMM, 2}, [0xE5] = {SBC, ZER, 3}, [0xF5] = {SBC, ZPX, 4}, [0xED] = {SBC, ABS, 4},
    [0xFD] = {SBC, ABX, 4}, [0xF9] = {SBC, ABY, 4}, [0xE1] = {SBC, IDX, 6}, [0xF1] = {SBC, IDY, 5},

    [0x38] = {SEC, IMP, 2}, [0xF8] = {SED, IMP, 2}, [0x78] = {SEI, IMP, 2},

    [0x85] = {STA, ZER, 3}, [0x95] = {STA, ZPX, 4}, [0x8D] = {STA, ABS, 4}, [0x9D] = {STA, ABX, 5},
    [0x99] = {STA, ABY, 5}, [0x81] = {STA, IDX, 6}, [0x91] = {STA, IDY, 6},

    [0x86] = {STX, ZER, 3}, [0x96] = {STX, ZPY, 4}, [0x8E] = {STX, ABS, 4},

    [0x84] = {STY, ZER, 3}, [0x94] = {STY, ZPX, 4}, [0x8C] = {STY, ABS, 4},

    [0xAA] = {TAX, IMP, 2}, [0xA8] = {TAY, IMP, 2}, [0xBA] = {TSX, IMP, 2}, [0x8A] = {TXA, IMP, 2},
    [0x9A] = {TXS, IMP, 2}, [0x98] = {TYA, IMP, 2},
  };
}  // namespace

}  // namespace nemu

#endif
