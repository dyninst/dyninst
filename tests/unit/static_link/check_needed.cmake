# Fails if EXE has a dynamic dependency on any of SONAMES ('|'-separated),
# the sonames of the shared Dyninst libraries.
execute_process(
  COMMAND ${OBJDUMP} -p ${EXE}
  OUTPUT_VARIABLE _out
  RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "objdump failed on ${EXE}")
endif()

string(REPLACE "|" ";" _sonames "${SONAMES}")
string(REGEX MATCHALL "NEEDED +[^\n]+" _needed "${_out}")
set(_found)
foreach(n ${_needed})
  string(REGEX REPLACE "^NEEDED +" "" n "${n}")
  list(FIND _sonames "${n}" _i)
  if(NOT _i EQUAL -1)
    list(APPEND _found ${n})
  endif()
endforeach()
if(_found)
  message(FATAL_ERROR "${EXE} needs shared Dyninst libraries: ${_found}")
endif()
