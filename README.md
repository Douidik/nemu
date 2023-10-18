# 👇 About nemu
Nemu is an open-source NES emulator written in modern C++.
![screenrecord](assets/nemu_screenrecord.gif)

## Features
- Hardware accurate emulation.
- Customizable configuration with .sd files.
- Support for MMC1 mapper games.
- Disassemble 6502 code.

## Getting started
```shell
git clone https://github.com/Douidik/nemu
cd nemu
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
cd ..
# The asset folder must be in the current working directory
./build/bin/nemu programmer ./<your_nes_file>
```
