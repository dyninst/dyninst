include_guard(GLOBAL)

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

# Check if the CMAKE_<LANG>_COMPILER supports static linking
set(_CMAKE_REQUIRED_LIBRARIES_BAK ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES "-static")

set(_src_code "int main(){ return 0; }")

check_c_source_compiles("${_src_code}" DYNINST_C_STATIC_LINK)
check_cxx_source_compiles("${_src_code}" DYNINST_CXX_STATIC_LINK)

set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_BAK})
