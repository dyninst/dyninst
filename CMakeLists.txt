cmake_minimum_required (VERSION 2.6.4)
project (Dyninst)

set (DYNINST_ROOT ${PROJECT_SOURCE_DIR})

include (${DYNINST_ROOT}/cmake/shared.cmake)

if(${PLATFORM} MATCHES nt)
set (HEADER_DIRS common
                 dataflowAPI
                 dyninstAPI
                 dynutil
                 instructionAPI
                 parseAPI
                 patchAPI
                 proccontrol
                 stackwalk
                 symtabAPI
    )
else()
set (HEADER_DIRS common
                 dataflowAPI
                 dwarf
                 dyninstAPI
                 dynutil
                 elf
                 instructionAPI
                 parseAPI
                 patchAPI
                 proccontrol
                 stackwalk
                 symlite
                 symtabAPI
    )
endif()

foreach (dir ${HEADER_DIRS})
  include_directories ( ${DYNINST_ROOT}/${dir}/h )
endforeach()

include_directories (
                    ${DYNINST_ROOT}
                    ${DYNINST_ROOT}/external
  )

# Component time
add_subdirectory (common)
if(NOT ${PLATFORM} MATCHES nt)
    add_subdirectory (elf)
    add_subdirectory (dwarf)
    add_subdirectory (symlite)
endif()
add_subdirectory (instructionAPI)
add_subdirectory (symtabAPI)
add_subdirectory (parseAPI)
add_subdirectory (patchAPI)
add_subdirectory (proccontrol)
add_subdirectory (stackwalk)
add_subdirectory (dyninstAPI)

#Build the RT library as a seperate project so we can change compilers
set(RT_SOURCE_DIR ${DYNINST_ROOT}/dyninstAPI_RT)
set(RT_BINARY_DIR ${PROJECT_BINARY_DIR}/dyninstAPI_RT)
file(MAKE_DIRECTORY ${RT_BINARY_DIR})
if (PLATFORM MATCHES bgq)
set (RT_C_COMPILER mpicc)
else()
set (RT_C_COMPILER ${CMAKE_C_COMPILER})
endif()

execute_process(WORKING_DIRECTORY ${RT_BINARY_DIR}
                COMMAND ${CMAKE_COMMAND}
                -DCMAKE_C_COMPILER=${RT_C_COMPILER}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                -DPLATFORM=${PLATFORM}
                ${RT_SOURCE_DIR})
add_custom_target(DyninstRT
                  ALL
                  COMMAND ${CMAKE_COMMAND} --build ${RT_BINARY_DIR} --config $<CONFIGURATION>
                  COMMENT "Building DyninstRT"
                  VERBATIM)
install(CODE "include(\"${RT_BINARY_DIR}/cmake_install.cmake\")")


if(UNIX)
add_dependencies(dynDwarf libdwarf_imp)
add_dependencies(dynElf libelf_imp)
if (NOT USE_GNU_DEMANGLER)
add_dependencies(common libiberty_imp)
endif()
add_dependencies(libdwarf_imp libelf_imp)
endif()

if(NOT ${PLATFORM} MATCHES nt)
    SET_TARGET_PROPERTIES (
        common dynElf dynDwarf instructionAPI symtabAPI symLite parseAPI
        patchAPI pcontrol stackwalk dyninstAPI
        PROPERTIES
            SOVERSION ${SOVERSION}
            VERSION ${LIBVERSION}
    )
else()
    SET_TARGET_PROPERTIES (
        common instructionAPI symtabAPI parseAPI
        patchAPI pcontrol stackwalk dyninstAPI
        PROPERTIES
            SOVERSION ${SOVERSION}
            VERSION ${LIBVERSION}
    )

endif()

if(UNIX)
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
install (FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfig.cmake"
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")
install (EXPORT DyninstTargets
        DESTINATION "${INSTALL_CMAKE_DIR}")

configure_file(cmake/DyninstConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfig.cmake" @ONLY)

# And version file
configure_file(cmake/DyninstConfigVersion.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfigVersion.cmake" @ONLY)
