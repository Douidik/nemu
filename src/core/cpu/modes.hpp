#ifndef NEMU_CPU_MODES_HPP
#define NEMU_CPU_MODES_HPP

namespace nemu::cpu {

enum Mode {
  ACC = 1 << 0x00,
  ABS = 1 << 0x01,
  ABX = 1 << 0x02,
  ABY = 1 << 0x03,
  IMM = 1 << 0x04,
  IMP = 1 << 0x05,
  IND = 1 << 0x06,
  IDX = 1 << 0x07,
  IDY = 1 << 0x08,
  REL = 1 << 0x09,
  ZER = 1 << 0x0A,
  ZPX = 1 << 0x0B,
  ZPY = 1 << 0x0C,

  MEMORY = ABS | ABX | ABY | IND | IDX | IDY | ZER | ZPX | ZPY,
};

}  // namespace nemu::modes

#endif
