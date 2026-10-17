include_guard(GLOBAL)

# A program linked against the static libraries must not also load the
# shared ones: two copies of a Dyninst library in one process each have
# their own global state.

find_program(DYNINST_OBJDUMP NAMES objdump)
if(NOT DYNINST_OBJDUMP)
  message(FATAL_ERROR "objdump is required for the static_link tests")
endif()

add_executable(static_link static_link.cpp)
target_compile_options(static_link PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(static_link PRIVATE parseAPI_static)
# Keep every linked shared library in NEEDED, so the check sees what was linked.
target_link_options(static_link PRIVATE "LINKER:--no-as-needed")

add_test(NAME static_link_no_shared_dyninst
         COMMAND ${CMAKE_COMMAND} -DOBJDUMP=${DYNINST_OBJDUMP}
                 -DEXE=$<TARGET_FILE:static_link> -P
                 ${CMAKE_CURRENT_SOURCE_DIR}/check_needed.cmake)
set_tests_properties(static_link_no_shared_dyninst PROPERTIES LABELS "unit")
