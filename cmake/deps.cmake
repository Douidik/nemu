set(CPM_VERSION 0.35.0)
set(CPM_FILEPATH "${NEMU_ROOT}/cmake/cpm.cmake")

if(NOT EXISTS ${CPM_SOURCE_FILEPATH})
  file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake ${CPM_FILEPATH})
endif()

include(${CPM_FILEPATH})

cpmaddpackage("gh:douidik/sdata#v1.0.8")
cpmaddpackage("gh:fmtlib/fmt#8.1.1")
cpmaddpackage("gh:libsdl-org/SDL#release-2.28.4")
cpmaddpackage("gh:catchorg/catch2#v2.13.8")
