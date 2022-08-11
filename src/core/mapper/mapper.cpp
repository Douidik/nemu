#include "mapper.hpp"
#include "exception.hpp"
#include "mapper_mmc1.hpp"
#include "mapper_nrom.hpp"

namespace nemu {

std::shared_ptr<Mapper> Mapper::create(Rom &rom) {
  uint8 type = rom.meta.mapper_upper << 4 | rom.meta.mapper_lower;

  switch (type) {
  case 0: return std::make_shared<MapperNRom>(rom);
  case 1: return std::make_shared<MapperMmc1>(rom);
  }

  throw Exception {"Rom Mapper #{} is not supported", type};
}

}  // namespace nemu
