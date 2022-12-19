set(BUILD_SHARED_LIBS ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
     isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if(LIGHTWEIGHT_SYMTAB)
    set(SYMREADER symLite)
else()
    set(SYMREADER symtabAPI)
endif()

# Link libraries privately when possible
function(target_link_private_libraries target)
    if(${CMAKE_VERSION} VERSION_LESS "2.8.7")
        target_link_libraries(${target} ${ARGN})
    else()
        target_link_libraries(${target} LINK_PRIVATE ${ARGN})
    endif()
endfunction()

set(ALL_DYNINST_TARGETS
    ""
    CACHE INTERNAL "")

function(dyninst_library target)
    add_library(${target} ${SRC_LIST})
    target_link_private_libraries(${target} ${ARGN})
    file(GLOB headers "h/*.h" "${CMAKE_CURRENT_BINARY_DIR}/h/*.h")
    set(ACTUAL_TARGETS ${target})
    set(ALL_TARGETS "${ARGN};${target}")
    if(${ENABLE_STATIC_LIBS})
        set(ACTUAL_TARGETS ${ACTUAL_TARGETS} ${target}_static)
        add_library(${target}_static STATIC ${SRC_LIST})
    endif()
    message(STATUS "Building ${ACTUAL_TARGETS}...")
    set_target_properties(${ACTUAL_TARGETS} PROPERTIES PUBLIC_HEADER "${headers}")
    set_target_properties(${ACTUAL_TARGETS} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                                       ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(
        ${target}
        PROPERTIES SOVERSION ${SOVERSION}
                   VERSION ${LIBVERSION}
                   CLEAN_DIRECT_OUTPUT 1)
    set(INSTALL_TARGETS ${ACTUAL_TARGETS})
    foreach(dep ${ARGN})
        message(STATUS "Processing dependent target ${dep}...")
        if(TARGET ${dep})
            get_target_property(dep_dir ${dep} LIBRARY_OUTPUT_DIRECTORY)
            if(EXISTS ${dep_dir} AND IS_DIRECTORY ${dep_dir})
                message(STATUS "Found dependency location ${dep_dir}")
                install(SCRIPT ${dep_dir}/cmake_install.cmake)
            endif()
        endif()
    endforeach()
    install(
        TARGETS ${INSTALL_TARGETS}
        EXPORT ${target}Targets
        COMPONENT ${target}
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
    add_custom_target(
        ${target}-install
        DEPENDS ${target}
        COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake")
    set(ALL_DYNINST_TARGETS
        "${ALL_DYNINST_TARGETS};${target}"
        CACHE INTERNAL "")
    install(EXPORT ${target}Targets DESTINATION "${INSTALL_CMAKE_DIR}")
    configure_file(
        "${DYNINST_ROOT}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
        @ONLY)
    configure_file(
        "${DYNINST_ROOT}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in"
        "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake"
        @ONLY)
    install(
        FILES
            "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
            "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")
endfunction()

set(INSTALL_BIN_DIR
    bin
    CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR
    lib
    CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR
    include
    CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR
    lib/cmake/${PROJECT_NAME}
    CACHE PATH "Installation directory for CMake files")

# Make the above absolute paths if necessary
foreach(p BIN LIB INCLUDE CMAKE)
    set(var INSTALL_${p}_DIR)
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
        add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
    else()
        add_definitions(-Dsnprintf=_snprintf)
    endif()
endif()
