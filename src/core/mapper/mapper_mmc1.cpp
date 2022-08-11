#include "mapper_mmc1.hpp"

namespace nemu {

void MapperMmc1::init() {
  m_control = 0x1C, m_buffer = 0x00, m_shift = 0x00;
  m_program_bank[0] = 0x00, m_program_bank[1] = m_rom.meta.prg_pages - 1;
  m_character_bank[0] = 0x00, m_character_bank[1] = 0x00;
}

Mirror MapperMmc1::mirror() const {
  uint8 mirror = m_control & 2;

  switch (mirror) {
  case 0: return Mirror::ONE_SCREEN_LO;
  case 1: return Mirror::ONE_SCREEN_UP;
  case 2: return Mirror::VERTICAL;
  case 3: return Mirror::HORIZONTAL;
  }

  return {};
}

uint32 MapperMmc1::map_prg(uint16 n) const {
  uint8 bank_mode = (m_control >> 2) & 0b1'1;

  switch (bank_mode) {
  case 0b0'0:
  case 0b0'1: {
    return (m_program_bank[0] & 0b1111'1110) * 0x8000 | (n & 0x7FFF);
  }

  case 0b1'0: {
    switch (n) {
    case 0x8000 ... 0xBFFF: return (n & 0x3FFF);
    case 0xC000 ... 0xFFFF: return (m_program_bank[0] & 0b0000'1111) * 0x4000 | (n & 0x3FFF);
    }
  }

  case 0b1'1: {
    switch (n) {
    case 0x8000 ... 0xBFFF: return (m_program_bank[0] & 0b0000'1111) * 0x4000 | (n & 0x3FFF);
    case 0xC000 ... 0xFFFF: return (m_program_bank[1] & 0b0000'1111) * 0x4000 | (n & 0x3FFF);
    }
  }
  }

  return {};
}

uint32 MapperMmc1::map_chr(uint16 n) const {
  uint8 bank_mode = (m_control >> 4) & 1;

  switch (bank_mode) {
  case 0b0: {
    return (m_character_bank[0] & 0b0001'1110) * 0x2000 | (n & 0x1FFF);
  }

  case 0b1: {
    switch (n) {
    case 0x0000 ... 0x0FFF: return (m_character_bank[0] & 0b0001'1111) * 0x1000 | (n & 0x0FFF);
    case 0x1000 ... 0x1FFF: return (m_character_bank[1] & 0b0001'1111) * 0x1000 | (n & 0x0FFF);
    }
  }
  }

  return {};
}

uint16 MapperMmc1::map_ram(uint16 n) const {
  return n & 0x1FFF;
}

uint8 *MapperMmc1::cpu_write(uint16 n, uint8 data) {
  switch (n) {
  case 0x6000 ... 0x7FFF: {
    return &(m_ram[map_ram(n)] = data);
  }

  case 0x8000 ... 0xFFFF: {
    if (data & 0b1000'0000) {
      return &(m_control |= 0x0C, m_buffer = 0x00, m_shift = 0x00);
    }

    m_buffer = ((data & 0b1) << 4) | (m_buffer >> 1), m_shift++;

    if (m_shift > 4) {
      // Write the buffer into the register and reset it
      uint8 buffer = m_buffer;
      m_buffer = 0x00, m_shift = 0x00;

      switch (n) {
      case 0x8000 ... 0x9FFF: {
        return &(m_control = buffer);
      }

      case 0xA000 ... 0xBFFF: {
        return &(m_character_bank[0] = buffer);
      }

      case 0xC000 ... 0xDFFF: {
        return &(m_character_bank[1] = buffer);
      }

      case 0xE000 ... 0xFFFF: {
        return &(m_program_bank[0] = buffer);
      }
      }
    }

    return &m_buffer;
  }
  }

  return nullptr;
}

const uint8 *MapperMmc1::cpu_peek(uint16 n) const {
  switch (n) {
  case 0x6000 ... 0x7FFF: {
    return &m_ram[map_ram(n)];
  }

  case 0x8000 ... 0xFFFF: {
    return &m_rom.program[map_prg(n)];
  }
  }

  return nullptr;
}

uint8 *MapperMmc1::cpu_read(uint16 n) {
  switch (n) {
  case 0x6000 ... 0x7FFF: {
    return &m_ram[map_ram(n)];
  }

  case 0x8000 ... 0xFFFF: {
    return &m_rom.program[map_prg(n)];
  }
  }

  return nullptr;
}

uint8 *MapperMmc1::ppu_write(uint16 n, uint8 data) {
  if (n > 0x1FFF) {
    return nullptr;
  }

  if (!m_rom.meta.chr_pages) {
    return &(m_ram[map_chr(n)] = data);
  } else {
    return &(m_rom.character[map_chr(n)] = data);
  }
}

const uint8 *MapperMmc1::ppu_peek(uint16 n) const {
  if (n > 0x1FFF) {
    return nullptr;
  }

  if (!m_rom.meta.chr_pages) {
    return &(m_ram[map_chr(n)]);
  } else {
    return &(m_rom.character[map_chr(n)]);
  }
}

uint8 *MapperMmc1::ppu_read(uint16 n) {
  if (n > 0x1FFF) {
    return nullptr;
  }

  if (!m_rom.meta.chr_pages) {
    return &(m_ram[map_chr(n)]);
  } else {
    return &(m_rom.character[map_chr(n)]);
  }
}

std::span<const uint8> MapperMmc1::pattern(uint8 n) const {
  return {ppu_peek(n * 0x1000), 0x1000};
}

}  // namespace nemu
