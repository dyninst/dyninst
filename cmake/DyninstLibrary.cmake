set(ALL_DYNINST_TARGETS
    ""
    CACHE INTERNAL "")

function(dyninst_library target)
    add_library(${target} ${SRC_LIST})
    target_link_libraries(${target} PRIVATE ${ARGN})
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
        PROPERTIES SOVERSION ${DYNINST_SOVERSION}
                   VERSION ${DYNINST_LIBVERSION}
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
    set(ALL_DYNINST_TARGETS
        "${ALL_DYNINST_TARGETS};${target}"
        CACHE INTERNAL "")
    install(EXPORT ${target}Targets DESTINATION "${INSTALL_CMAKE_DIR}")
endfunction()

if(DYNINST_OS_Windows)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
        add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
    else()
        add_definitions(-Dsnprintf=_snprintf)
    endif()
endif()
