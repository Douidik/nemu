file(
  GLOB_RECURSE NEMU_CORE_SOURCE
  ${NEMU_SOURCE_REGEX}*.hpp
  ${NEMU_SOURCE_REGEX}*.cpp
)

add_library(nemu_core ${NEMU_CORE_SOURCE})

target_include_directories(
  nemu_core PRIVATE
  ${NEMU_ROOT}/src/core/
)

target_link_libraries(
  nemu_core PUBLIC
  fmt::fmt
  sdata
)

set_target_properties(
  nemu_core PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
