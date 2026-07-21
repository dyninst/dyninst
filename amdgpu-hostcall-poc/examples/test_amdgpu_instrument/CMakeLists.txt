project(test_amdgpu_instrument LANGUAGES CXX)

# Full capability demonstrator (env-gated test harness).
add_executable(test_amdgpu_instrument test_amdgpu_instrument.C)
target_compile_options(test_amdgpu_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(test_amdgpu_instrument PRIVATE Dyninst::dyninstAPI)

# Minimal, single-purpose example of the intended user API (record EXEC at a few sites).
add_executable(minimal_instrument minimal_instrument.C)
target_compile_options(minimal_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(minimal_instrument PRIVATE Dyninst::dyninstAPI)

# User-library model: insert calls to a user-written probe (compiled together with
# the hostcall runtime into one code object; see user_lib/).
add_executable(user_probe_instrument user_probe_instrument.C)
target_compile_options(user_probe_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(user_probe_instrument PRIVATE Dyninst::dyninstAPI)

# Per-wave variable: pass THIS wave's slice of a per-wave buffer as a pointer arg.
add_executable(per_wave_instrument per_wave_instrument.C)
target_compile_options(per_wave_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(per_wave_instrument PRIVATE Dyninst::dyninstAPI)

# Per-wave variable that HOLDS a call's return value (capture) + passes it as an arg.
add_executable(capture_return_instrument capture_return_instrument.C)
target_compile_options(capture_return_instrument PRIVATE ${EXAMPLES_WARNING_FLAGS})
target_link_libraries(capture_return_instrument PRIVATE Dyninst::dyninstAPI)

