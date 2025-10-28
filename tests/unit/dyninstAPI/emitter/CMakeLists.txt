include_guard(GLOBAL)

add_library(emitter_test INTERFACE)
target_sources(emitter_test INTERFACE emitter_test.h)

if(DYNINST_CODEGEN_ARCH_X86_64)
  add_executable(x86_64 x86_64.cpp)
  target_compile_options(x86_64 PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_compile_definitions(x86_64 PRIVATE ${DYNINST_PLATFORM_CAPABILITIES})
  target_link_libraries(x86_64 PRIVATE emitter_test dyninstAPI)
  add_test(NAME dyninstAPI_emitter_x86_64 COMMAND x86_64)
endif()
