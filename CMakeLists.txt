cmake_minimum_required(VERSION 3.18)

include(cmake/conf.cmake)
include(cmake/deps.cmake)

project(
  nemu
  DESCRIPTION "Modern Open-source NES emulator"
  LANGUAGES CXX
)

project(
  nemu-core
  DESCRIPTION "Nemu emulator core"
  LANGUAGES CXX
)

project(
  nemu-test
  DESCRIPTION "Nemu test suite"
  LANGUAGES CXX
)

add_subdirectory(src/nemu/)
add_subdirectory(src/core/)
add_subdirectory(src/test/)
