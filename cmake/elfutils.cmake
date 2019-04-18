############################################
# elfutils.cmake
#
# Configure libelf and libdwarf for Dyninst
#
############################################

if(NOT UNIX)
  return()
endif()

find_package(LibDwarf)
find_package(LibElf 0.173)

if(NOT LIBELF_FOUND OR NOT LIBDWARF_FOUND)
  message(STATUS "Attempting to build elfutils as external project")
  include(ExternalProject)
  externalproject_add(
    LibElf
    PREFIX ${CMAKE_BINARY_DIR}/elfutils
    URL https://sourceware.org/elfutils/ftp/elfutils-latest.tar.bz2
    CONFIGURE_COMMAND
      CFLAGS=-g
      <SOURCE_DIR>/configure
      --enable-install-elfh
      --enable-shared
      --prefix=${CMAKE_BINARY_DIR}/elfutils
    BUILD_COMMAND make
    INSTALL_COMMAND make install
  )
  set(LIBELF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
  set(LIBELF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libelf.so)
  set(LIBDWARF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
  set(LIBDWARF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libdw.so)
  set(SHOULD_INSTALL_LIBELF 1)
else()
  set(SHOULD_INSTALL_LIBELF 0)
endif()

add_library(libelf_imp SHARED IMPORTED)
set_property(TARGET libelf_imp PROPERTY IMPORTED_LOCATION ${LIBELF_LIBRARIES})
if(NOT LIBELF_FOUND)
  add_dependencies(libelf_imp LibElf)
endif()
add_library(libdwarf_imp SHARED IMPORTED)
set_property(TARGET libdwarf_imp
             PROPERTY IMPORTED_LOCATION ${LIBDWARF_LIBRARIES})
if(NOT LIBDWARF_FOUND)
  add_dependencies(libdwarf_imp LibDwarf)
endif()

if(UNIX)
  message(STATUS "Adding Unix-specific dependencies")
  add_dependencies(dynDwarf libdwarf_imp)
  add_dependencies(dynElf libelf_imp)
  message(STATUS "Added libdwarf_imp and libelf_imp dependencies")
  if(NOT USE_GNU_DEMANGLER)
    add_dependencies(common libiberty_imp)
  endif()
  if(NOT LIBDWARF_FOUND)
    message(STATUS "Building libdwarf, creating libdwarf dependency on libelf")
    add_dependencies(libdwarf_imp libelf_imp)
  endif()
  if(SHOULD_INSTALL_LIBELF)
    get_filename_component(ELFDIR "${LIBELF_LIBRARIES}" PATH)
    file(GLOB LIBELF_INSTALL_FILES ${ELFDIR}/libelf*.so* ${ELFDIR}/libdw*.so*)
    message(STATUS "Libelf is ${LIBELF_LIBRARIES}")
    message(STATUS "Installing ${LIBELF_INSTALL_FILES} from ${ELFDIR}")
    install(FILES ${LIBELF_INSTALL_FILES} DESTINATION "${INSTALL_LIB_DIR}")
  endif()
  if(NOT LIBDWARF_FOUND)
    message(STATUS "Installing ${LIBDWARF_LIBRARIES}")
    install(FILES ${LIBDWARF_LIBRARIES} DESTINATION "${INSTALL_LIB_DIR}")
  endif()
endif()
