include_guard(GLOBAL)

# A program linked against the static libraries must not also load the
# shared ones: two copies of a Dyninst library in one process each have
# their own global state.

get_property(_libs GLOBAL PROPERTY DYNINST_LIBRARIES)

set(_sonames)
foreach(l ${_libs})
  list(APPEND _sonames "$<TARGET_SONAME_FILE_NAME:${l}>")
endforeach()
string(JOIN "|" _sonames ${_sonames})

# One test for every library, so every configuration has the same tests: skipped without
# its static variant, or without objdump.
foreach(l ${_libs})
  set(_requires objdump static_lib:${l})
  dyninst_add_test_executable(
    static_link_${l}
    SOURCE_FILES static_link.cpp
    DEPS ${l}_static
    REQUIRES ${_requires}
  )
  if(TARGET static_link_${l})
    # Keep every linked shared library in NEEDED, so the check sees what was linked.
    # The probe uses no Dyninst symbol, so nothing defines the ps_* callbacks that
    # libthread_db expects from its user; it is never run, so allow them undefined.
    target_link_options(
      static_link_${l}
      PRIVATE "LINKER:--no-as-needed" "LINKER:--allow-shlib-undefined"
    )
  endif()

  dyninst_add_test(
    static_link_${l}
    TYPE unit
    COMMAND
      ${CMAKE_COMMAND} -DOBJDUMP=${CMAKE_OBJDUMP} -DEXE=$<TARGET_FILE:static_link_${l}>
      -DSONAMES=${_sonames} -P ${CMAKE_CURRENT_SOURCE_DIR}/check_needed.cmake
    REQUIRES ${_requires}
  )
endforeach()
