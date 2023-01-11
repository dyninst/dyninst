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
    set(LTO_LINK_FLAGS "-fuse-ld=bfd")
  endif()
	list(APPEND DYNINST_FLAGS_DEBUG   "-Og -g3")
	list(APPEND DYNINST_FLAGS_RELEASE "-O2 -g3")
	list(APPEND DYNINST_FLAGS_RELWITHDEBINFO  "-O2 -g3")
	list(APPEND DYNINST_FLAGS_MINSIZEREL  "-Os")

  set(FORCE_FRAME_POINTER "-fno-omit-frame-pointer")
  # Ensure each library is fully linked
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LTO_LINK_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LTO_LINK_FLAGS}")
else(MSVC)
	list(APPEND DYNINST_FLAGS_DEBUG "/MP /Od /Zi /MDd /D_DEBUG")
	list(APPEND DYNINST_FLAGS_RELEASE "/MP /O2 /MD")
	list(APPEND DYNINST_FLAGS_RELWITHDEBINFO "/MP /O2 /Zi /MD")
	list(APPEND DYNINST_FLAGS_MINSIZEREL"/MP /O1 /MD")

  set(FORCE_FRAME_POINTER "/Oy-")

  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LTO_LINK_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LTO_LINK_FLAGS}")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${LTO_LINK_FLAGS}")
endif()
message(STATUS "Set optimization flags")
