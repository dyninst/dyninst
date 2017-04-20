cmake_minimum_required (VERSION 2.6.4)
project (Dyninst)

set (DYNINST_ROOT ${PROJECT_SOURCE_DIR})
set (CMAKE_SKIP_BUILD_RPATH FALSE)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES 
	  "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
		    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF()

set(RT_SOURCE_DIR ${DYNINST_ROOT}/dyninstAPI_RT)
set(RT_BINARY_DIR ${PROJECT_BINARY_DIR}/dyninstAPI_RT)

include (${DYNINST_ROOT}/cmake/shared.cmake)

configure_file(cmake/version.h.in common/h/version.h)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_BINARY_DIR}/common/h)
set (HEADER_DIRS common
    dataflowAPI
    dyninstAPI
    instructionAPI
    parseAPI
    patchAPI
    proccontrol
    stackwalk
    symtabAPI
    )
if(NOT ${PLATFORM} MATCHES nt)
  set (HEADER_DIRS ${HEADER_DIRS}
    dwarf
    elf
    symlite
    )
endif()

foreach (dir ${HEADER_DIRS})
  include_directories ( ${DYNINST_ROOT}/${dir}/h )
endforeach()

include_directories (
  ${DYNINST_ROOT}
  ${DYNINST_ROOT}/external
  )
set(Boost_USE_STATIC_LIBS OFF)
# Component time
add_subdirectory (common)
if(NOT ${PLATFORM} MATCHES nt)
  add_subdirectory (elf)
  add_subdirectory (dwarf)
  add_subdirectory (symlite)
  add_subdirectory (parseThat)
endif()
add_subdirectory (instructionAPI)
add_subdirectory (symtabAPI)
add_subdirectory (parseAPI)
add_subdirectory (proccontrol)
add_subdirectory (stackwalk)
add_subdirectory (patchAPI)
if(${SYMREADER} MATCHES symtabAPI)
  add_subdirectory (dyninstAPI)
  add_subdirectory (dynC_API)
endif()

if(BUILD_RTLIB)
  # Build the RT library as a separate project so we can change compilers
  message(STATUS "Configuring DyninstAPI_RT")
  file(REMOVE_RECURSE ${RT_BINARY_DIR}/CMakeCache.txt ${RT_BINARY_DIR}/CMakeFiles ${RT_BINARY_DIR}/Makefile)
  file(MAKE_DIRECTORY ${RT_BINARY_DIR})
  if (PLATFORM MATCHES bgq)
    set (RT_C_COMPILER mpicc CACHE STRING "Compiler for runtime library")
  else()
    set (RT_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING "Compiler for runtime library")
  endif()
  set (ENABLE_STATIC_LIBS NO CACHE STRING "Build static libraries as well?")
  message(STATUS "Configuring RT library")

  execute_process(WORKING_DIRECTORY ${RT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND}
    -DCMAKE_C_COMPILER=${RT_C_COMPILER}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DINSTALL_LIB_DIR=${INSTALL_LIB_DIR}
    -DINSTALL_INCLUDE_DIR=${INSTALL_INCLUDE_DIR}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DBUILD_RTLIB_32=${BUILD_RTLIB_32}
    -DPLATFORM=${PLATFORM}
    -G ${CMAKE_GENERATOR}
    ${RT_SOURCE_DIR})

  if(MSVC)
      include_external_msproject(DyninstAPI_RT dyninstAPI_RT/dyninstAPI_RT.vcxproj)
	  include_external_msproject(DyninstAPI_RT_static dyninstAPI_RT/dyninstAPI_RT_static.vcxproj)
  else()
    add_custom_target(DyninstRT
      ALL
      $(MAKE)
      WORKING_DIRECTORY ${RT_BINARY_DIR}
      COMMENT "Building DyninstRT")
    add_dependencies(dyninstAPI DyninstRT)
    if(TARGET dyninstAPI-static)
      add_dependencies(dyninstAPI-static DyninstRT)
    endif()
  endif()

  install(SCRIPT "${RT_BINARY_DIR}/cmake_install.cmake")

else()
  message(STATUS "Skipping DyninstAPI_RT. Be sure to build this library if you're using instrumentation.")
endif()
set (VERSION_STRING "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}.${DYNINST_PATCH_VERSION}")
set (DYNINST_NAME "DyninstAPI-${VERSION_STRING}")

if(BUILD_TARBALLS)
  find_package(Git)
  if(GIT_FOUND)
    if(EXISTS "${DYNINST_ROOT}/.git/")
      message(STATUS "Source tree is repository, building archive target")
      add_custom_target(package ALL)
      add_custom_command(TARGET package
        COMMAND ${GIT_EXECUTABLE} archive --prefix="${DYNINST_NAME}/" --format=tar.gz -o "${CMAKE_BINARY_DIR}/${DYNINST_NAME}.tgz" HEAD
        WORKING_DIRECTORY ${DYNINST_ROOT}
        COMMENT "Packaging Dyninst")
    endif()
  endif()
endif()
if(BUILD_DOCS)
  add_custom_target(doc)
  set(LATEX_DOCS dynC_API dataflowAPI instructionAPI parseAPI patchAPI symtabAPI stackwalk)
  set(WORD_DOCS proccontrol dyninstAPI)
  add_custom_target(proccontrol-doc
    DEPENDS ${CMAKE_SOURCE_DIR}/proccontrol/doc/proccontrol.pdf)
  add_custom_target(dyninstAPI-doc
    DEPENDS ${CMAKE_SOURCE_DIR}/dyninstAPI/doc/dyninstAPI.pdf)
  foreach(COMPONENT ${WORD_DOCS})
    add_dependencies(doc ${COMPONENT}-doc)
    set_target_properties(${COMPONENT}-doc PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
    install(FILES ${CMAKE_SOURCE_DIR}/${COMPONENT}/doc/${COMPONENT}.pdf
      DESTINATION ${INSTALL_DOC_DIR}
      RENAME ${COMPONENT}-${VERSION_STRING}.pdf
      OPTIONAL
      )
  endforeach()

  find_package(LATEX)
  if(PDFLATEX_COMPILER)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/common/doc
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/common)
    foreach(COMPONENT ${LATEX_DOCS})
      file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${COMPONENT}/doc
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT})
      set (DEPS "")
      file(GLOB_RECURSE DEPS ${CMAKE_CURRENT_SOURCE_DIR}/${COMPONENT}/doc/*.tex ${CMAKE_CURRENT_SOURCE_DIR}/common/doc/*.tex)
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.aux
        DEPENDS ${DEPS}
        COMMAND ${PDFLATEX_COMPILER}
        ARGS -interaction=batchmode ${COMPONENT}.tex
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc
        COMMENT "Latex (first pass)"
        )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.log
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.aux
        COMMAND ${PDFLATEX_COMPILER}
        ARGS -interaction=batchmode ${COMPONENT}.tex
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc
        COMMENT "Latex (second pass)"
        )
      add_custom_target(${COMPONENT}-doc echo
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.log
        )
      add_dependencies(doc ${COMPONENT}-doc)
      set_target_properties(${COMPONENT}-doc PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
      install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.pdf
        DESTINATION ${INSTALL_DOC_DIR}
        RENAME ${COMPONENT}-${VERSION_STRING}.pdf
        OPTIONAL
        )
    endforeach()
  else()
    message(STATUS "LaTeX not found")
  endif()
endif()

if(UNIX)
  message(STATUS "Adding Unix-specific dependencies")
  add_dependencies(dynDwarf libdwarf_imp)
  add_dependencies(dynElf libelf_imp)
  message(STATUS "Added libdwarf_imp and libelf_imp dependencies")
  if (NOT USE_GNU_DEMANGLER)
    add_dependencies(common libiberty_imp)
  endif()
  if(NOT LIBDWARF_FOUND)
    message(STATUS "Building libdwarf, creating libdwarf dependency on libelf")
    add_dependencies(libdwarf_imp libelf_imp)
  endif()
  if(SHOULD_INSTALL_LIBELF)
    get_filename_component(ELFDIR "${LIBELF_LIBRARIES}" PATH)
    file(GLOB LIBELF_INSTALL_FILES ${ELFDIR}/libelf.so*)
    message(STATUS "Libelf is ${LIBELF_LIBRARIES}")
    message(STATUS "Installing ${LIBELF_INSTALL_FILES} from ${ELFDIR}")
    install (FILES ${LIBELF_INSTALL_FILES}
      DESTINATION "${INSTALL_LIB_DIR}")
  endif()
  if(NOT LIBDWARF_FOUND)
    message(STATUS "Installing ${LIBDWARF_LIBRARIES}")
    install (FILES ${LIBDWARF_LIBRARIES}
      DESTINATION "${INSTALL_LIB_DIR}")
  endif()
endif()

if(TARGET boost)
    add_dependencies(common boost)
endif()
