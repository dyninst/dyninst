include_guard(GLOBAL)

# A program linked against the static libraries must not also load the
# shared ones: two copies of a Dyninst library in one process each have
# their own global state.

if(NOT CMAKE_OBJDUMP)
  message(STATUS "objdump not found; not adding the static_link tests")
  return()
endif()

get_property(_libs GLOBAL PROPERTY DYNINST_LIBRARIES)

set(_sonames)
foreach(l ${_libs})
  list(APPEND _sonames "$<TARGET_SONAME_FILE_NAME:${l}>")
endforeach()
string(JOIN "|" _sonames ${_sonames})

foreach(l ${_libs})
  if(NOT TARGET ${l}_static)
    continue()
  endif()

  add_executable(static_link_${l} static_link.cpp)
  target_link_libraries(static_link_${l} PRIVATE ${l}_static)
  # Keep every linked shared library in NEEDED, so the check sees what was linked.
  # The probe uses no Dyninst symbol, so nothing defines the ps_* callbacks that
  # libthread_db expects from its user; it is never run, so allow them undefined.
  target_link_options(static_link_${l} PRIVATE "LINKER:--no-as-needed"
                      "LINKER:--allow-shlib-undefined")

  add_test(
    NAME static_link_${l}
    COMMAND
      ${CMAKE_COMMAND} -DOBJDUMP=${CMAKE_OBJDUMP} -DEXE=$<TARGET_FILE:static_link_${l}>
      -DSONAMES=${_sonames} -P ${CMAKE_CURRENT_SOURCE_DIR}/check_needed.cmake)
  set_tests_properties(static_link_${l} PROPERTIES LABELS "unit")
endforeach()
