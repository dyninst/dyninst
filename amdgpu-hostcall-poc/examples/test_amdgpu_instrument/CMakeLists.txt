project(test_amdgpu_instrument LANGUAGES CXX)

# The main code coverage tool
add_executable(test_amdgpu_instrument test_amdgpu_instrument.C)
target_compile_options(test_amdgpu_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(test_amdgpu_instrument PRIVATE Dyninst::dyninstAPI)

