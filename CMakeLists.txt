cmake_minimum_required (VERSION 3.4.0)
project (Dyninst)

set (DYNINST_ROOT ${PROJECT_SOURCE_DIR})
set (CMAKE_SKIP_BUILD_RPATH FALSE)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(STERILE_BUILD OFF CACHE BOOL "Do not download/build any third-party dependencies from source")

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES 
	  "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
		    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF()

set(RT_SOURCE_DIR ${DYNINST_ROOT}/dyninstAPI_RT)
set(RT_BINARY_DIR ${PROJECT_BINARY_DIR}/dyninstAPI_RT)

set (CMAKE_MODULE_PATH "${DYNINST_ROOT}/cmake" "${DYNINST_ROOT}/cmake/Modules" ${CMAKE_MODULE_PATH})

# Set the C and C++ language standards
include(LanguageStandards)

# Find the necessary third-party libraries
find_package(ThreadDB)
find_package(Threads)
include(Boost)
include(ThreadingBuildingBlocks)
include(ElfUtils)

if(UNIX)
  include(LibIberty REQUIRED)
endif()

include(shared)

if(USE_OpenMP)
  find_package(OpenMP REQUIRED)
endif()

configure_file(cmake/version.h.in common/h/dyninstversion.h)
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

set(ADD_VALGRIND_ANNOTATIONS OFF CACHE BOOL "Enable annotations for Valgrind analysis")
if(ADD_VALGRIND_ANNOTATIONS)
  find_package(Valgrind REQUIRED)
  include_directories(${Valgrind_INCLUDE_DIRS})
  add_definitions(-DENABLE_VG_ANNOTATIONS)
endif()

include_directories (
  ${DYNINST_ROOT}
  ${DYNINST_ROOT}/external
  ${TBB_INCLUDE_DIRS}
  )

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
  message(STATUS "Configuring DyninstAPI_RT in ${RT_BINARY_DIR}")
  file(REMOVE_RECURSE ${RT_BINARY_DIR}/CMakeCache.txt ${RT_BINARY_DIR}/CMakeFiles ${RT_BINARY_DIR}/Makefile)
  file(MAKE_DIRECTORY ${RT_BINARY_DIR})
  set (RT_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING "Compiler for runtime library")
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
    -B ${RT_BINARY_DIR}
    -S ${RT_SOURCE_DIR})
  find_file(${RT_MAKEFILE} Makefile PATHS ${RT_BINARY_DIR} NO_DEFAULT_PATH)
  message(STATUS "RTlib Makefile: ${RT_MAKEFILE}")
  if(MSVC)
      include_external_msproject(DyninstAPI_RT dyninstAPI_RT/dyninstAPI_RT.vcxproj)
	  include_external_msproject(DyninstAPI_RT_static dyninstAPI_RT/dyninstAPI_RT_static.vcxproj)
  else()
    add_custom_target(DyninstRT
      ALL
      $(MAKE)
      WORKING_DIRECTORY ${RT_BINARY_DIR}
      COMMENT "Building DyninstRT")
    if(TARGET TBB)
      add_dependencies(DyninstRT TBB)
    endif()
    add_dependencies(dyninstAPI DyninstRT)
    if(TARGET dyninstAPI-static)
      add_dependencies(dyninstAPI-static DyninstRT)
    endif()
    if(TARGET boost)
      add_dependencies(DyninstRT boost)
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
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}/doc/${COMPONENT}.pdf
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

if(TARGET boost)
    add_dependencies(common boost)
endif()

#add_subdirectory(testsuite)

# Copy out all of the cmake files so they can be used by the Testsuite
install(DIRECTORY ${DYNINST_ROOT}/cmake/ DESTINATION ${INSTALL_CMAKE_DIR})
install(FILES ${PROJECT_BINARY_DIR}/CMakeCache.txt DESTINATION ${INSTALL_CMAKE_DIR})
