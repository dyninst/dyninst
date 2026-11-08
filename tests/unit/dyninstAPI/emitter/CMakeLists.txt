include_guard(GLOBAL)

add_library(emitter_test INTERFACE)
target_sources(emitter_test INTERFACE emitter_test.h)

# Each emitter is built only for its codegen architecture.
foreach(_arch i386 x86_64 amdgpu_gfx908 aarch64)
  set(_deps emitter_test dyninstAPI)
  if(_arch STREQUAL "aarch64")
    set(_deps dyninstAPI)
  endif()
  # The gfx908 byte-golden emitter is produced by the one gfx9-family codegen build, so its codegen
  # requirement is the family arch (amdgpu_gfx9), not the gfx908 gen name.
  set(_req_arch ${_arch})
  if(_arch STREQUAL "amdgpu_gfx908")
    set(_req_arch amdgpu_gfx9)
  endif()
  dyninst_add_test(
    dyninstAPI_emitter_${_arch}
    TYPE unit
    SOURCE_FILES ${_arch}.cpp
    DEPS ${_deps}
    DEFINES ${DYNINST_PLATFORM_CAPABILITIES}
    REQUIRES codegen:${_req_arch}
  )
endforeach()
