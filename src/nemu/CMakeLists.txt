file(
  GLOB_RECURSE NEMU_SOURCE
  ${NEMU_SOURCE_REGEX}*.hpp
  ${NEMU_SOURCE_REGEX}*.cpp
)

add_executable(nemu ${NEMU_SOURCE})

target_include_directories(
  nemu PRIVATE
  ${NEMU_ROOT}/src/core/
  ${NEMU_ROOT}/src/nemu/
)

target_link_libraries(
  nemu PUBLIC
  nemu_core
  SDL2
  fmt::fmt
  sdata
)

set_target_properties(
  nemu PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
