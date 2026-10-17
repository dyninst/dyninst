# Fails if EXE has a dynamic dependency on any shared Dyninst library.
execute_process(COMMAND ${OBJDUMP} -p ${EXE} OUTPUT_VARIABLE _out RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "objdump failed on ${EXE}")
endif()

set(_libs common instructionAPI symtabAPI symLite parseAPI patchAPI pcontrol
          stackwalk dyninstAPI)
string(REPLACE ";" "|" _libs "${_libs}")
string(REGEX MATCHALL "NEEDED +lib(${_libs})\\.so[^\n]*" _found "${_out}")
if(_found)
  message(FATAL_ERROR "${EXE} needs shared Dyninst libraries:\n${_found}")
endif()
