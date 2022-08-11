#include "rom.hpp"
#include "exception.hpp"
#include "mapper/mapper.hpp"
#include <assert.h>
#include <memory.h>

namespace nemu {

Rom::Rom(std::span<uint8> data) {
  std::memcpy(&meta, &data[0], sizeof(RomMeta));

  if (std::string_view {meta.magic, 4} != "NES\x1a") {
    throw Exception {"ROM does not follow the iNES standard"};
  }

  if (meta.version != 0) {
    throw Exception {"Nemu does not support iNES standard version #{}", meta.version};
  }

  auto rom_begin = data.begin() + (meta.has_trainer ? 528 : 16);

  program = {
    rom_begin,
    rom_begin + (meta.prg_pages * PRG_PAGE_SIZE),
  };

  character = {
    program.end(),
    program.end() + (meta.chr_pages * CHR_PAGE_SIZE),
  };
}

}  // namespace nemu
