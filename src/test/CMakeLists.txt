file(
  GLOB_RECURSE NEMU_TEST_SOURCE
  ${NEMU_SOURCE_REGEX}*.hpp
  ${NEMU_SOURCE_REGEX}*.cpp
)

add_executable(nemu_test ${NEMU_TEST_SOURCE})

target_include_directories(
  nemu_test PRIVATE
  ${NEMU_ROOT}/src/core/
  ${NEMU_ROOT}/test/
)

target_link_libraries(
  nemu_test PRIVATE
  nemu_core
  Catch2::Catch2
)

set_target_properties(
  nemu_test PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)
