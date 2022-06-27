set(CPM_VERSION 0.35.0)
set(CPM_FILEPATH "${NEMU_ROOT}/cmake/cpm_${CPM_VERSION}.cmake")

if(NOT EXISTS ${CPM_SOURCE_FILEPATH})
  file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake ${CPM_FILEPATH})
endif()

include(${CPM_FILEPATH})

CPMAddPackage("gh:douidik/sdata#v1.0.1")
CPMAddPackage("gh:fmtlib/fmt#8.1.1")
CPMAddPackage("gh:LIBSDL-ORG/SDL#2.0.22")
CPMAddPackage("gh:catchorg/catch2#v2.13.8")
CPMAddPackage("gh:neargye/magic_enum#v0.7.3")
