include_guard(GLOBAL)

add_executable(noreturn_check noreturn_check.cpp)
target_link_libraries(noreturn_check PRIVATE dyninstAPI parseAPI)

add_executable(noreturn_scan noreturn_scan.cpp)
target_link_libraries(noreturn_scan PRIVATE parseAPI symtabAPI instructionAPI)

# The warning flags of the libraries, so DYNINST_WARNINGS_AS_ERRORS covers the tools as well.
foreach(_tool noreturn_check noreturn_scan)
  target_compile_options(${_tool} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
endforeach()

install(
  TARGETS noreturn_check noreturn_scan
  RUNTIME DESTINATION "${DYNINST_INSTALL_BINDIR}"
)
install(PROGRAMS generate_noreturn_list.sh DESTINATION "${DYNINST_INSTALL_BINDIR}")
