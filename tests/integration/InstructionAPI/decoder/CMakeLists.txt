include_guard(GLOBAL)

add_library(cft_tests OBJECT cft_tests.cpp cft_tests.h)
target_compile_options(cft_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(cft_tests PRIVATE instructionAPI)
target_include_directories(cft_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(memory_tests OBJECT memory_tests.cpp memory_tests.h)
target_compile_options(memory_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(memory_tests PRIVATE instructionAPI)
target_include_directories(memory_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(register_tests OBJECT register_tests.cpp register_tests.h)
target_compile_options(register_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(register_tests PRIVATE instructionAPI)
target_include_directories(register_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(x86)
