#ifndef NEMU_CPU_MODES_HPP
#define NEMU_CPU_MODES_HPP

namespace nemu {

enum class Mode {
  ACC,
  ABS,
  ABX,
  ABY,
  IMM,
  IMP,
  IND,
  IDX,
  IDY,
  REL,
  ZER,
  ZPX,
  ZPY,
};

}

#endif
