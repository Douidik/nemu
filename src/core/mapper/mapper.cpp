#include "mapper.hpp"
#include "exception.hpp"
#include "mapper_nrom.hpp"

namespace nemu {

std::shared_ptr<Mapper> Mapper::create(uint8 type, uint8 program_pages, uint8 character_pages) {
  switch (type) {
  case 0: return std::make_shared<MapperNRom>(program_pages, character_pages);
  }

  throw Exception {"Rom Mapper #{} is not supported", type};
}

}  // namespace nemu
