include_guard(GLOBAL)

if(ENABLE_LTO)
	include(CheckIPOSupported)
	check_ipo_supported(LANGUAGES "C" "CXX")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Make sure we don't get something like CC=gcc CXX=clang++
if(NOT ${CMAKE_C_COMPILER_ID} STREQUAL ${CMAKE_CXX_COMPILER_ID})
	message(FATAL_ERROR "C and C++ compilers are not the same vendor")
endif()

set(_linux_compilers "GNU" "Clang" "Intel" "IntelLLVM")

if(${CMAKE_CXX_COMPILER_ID} IN_LIST _linux_compilers)
  if(ENABLE_LTO)
    list(APPEND DYNINST_LINK_FLAGS "-fuse-ld=bfd")
  endif()
  
  # Used in stackwalk
  set(FORCE_FRAME_POINTER "-fno-omit-frame-pointer")
  
	list(APPEND DYNINST_FLAGS_DEBUG   "-Og -g3 ${FORCE_FRAME_POINTER}")
	list(APPEND DYNINST_FLAGS_RELEASE "-O2 -g3")
	list(APPEND DYNINST_FLAGS_RELWITHDEBINFO  "-O2 -g3")
	list(APPEND DYNINST_FLAGS_MINSIZEREL  "-Os")

  # Ensure each library is fully linked
	list(APPEND DYNINST_LINK_FLAGS "-Wl,--no-undefined")
elseif(MSVC)
  set(FORCE_FRAME_POINTER "/Oy-")
  
	list(APPEND DYNINST_FLAGS_DEBUG "/MP /Od /Zi /MDd /D_DEBUG ${FORCE_FRAME_POINTER}")
	list(APPEND DYNINST_FLAGS_RELEASE "/MP /O2 /MD")
	list(APPEND DYNINST_FLAGS_RELWITHDEBINFO "/MP /O2 /Zi /MD")
	list(APPEND DYNINST_FLAGS_MINSIZEREL"/MP /O1 /MD")
else()
	message(FATAL_ERROR "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()
message(STATUS "Set optimization flags")
