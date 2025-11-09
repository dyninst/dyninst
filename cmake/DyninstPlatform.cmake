include_guard(GLOBAL)

# Equivalent to CMAKE_HOST_SYSTEM_NAME and `uname -s` on Unixes
cmake_host_system_information(RESULT _host_os QUERY OS_NAME)

set(_known_oses "Linux" "FreeBSD" "Windows")
if(NOT ${_host_os} IN_LIST _known_oses)
  message(FATAL_ERROR "Unsupported OS: '${_host_os}'")
endif()

# Equivalent to CMAKE_HOST_SYSTEM_PROCESSOR and `uname -m` on Unixes
cmake_host_system_information(RESULT _host_arch QUERY OS_PLATFORM)

set(_32bit_x86_arches "i386" "i686")
set(_64bit_x86_arches "x86_64" "amd64")
set(_amdgpu_arches "amdgpu_gfx908" "amdgpu_gfx90a" "amdgpu_gfx940")
set(_known_arches "ppc64le" "aarch64" "riscv64" ${_32bit_x86_arches} ${_64bit_x86_arches}
                  ${_amdgpu_arches})

if(NOT ${_host_arch} IN_LIST _known_arches)
  message(FATAL_ERROR "Unsupported architecture: '${_host_arch}'")
endif()

set(_is_32bit_x86 FALSE)
if(_host_arch IN_LIST _32bit_x86_arches)
  set(_is_32bit_x86 TRUE)
endif()

# Equivalent to checking CMAKE_SIZEOF_VOID_P
cmake_host_system_information(RESULT _is64bit QUERY IS_64BIT)

# 32-bit is only supported on 32-bit x86
if(NOT _is64bit AND NOT ${_is_32bit_x86})
  message(FATAL_ERROR "32-bit programming is only supported on ${_32bit_x86_arches}")
endif()

# These checks are redundant, but protect against string name changes
if(${_host_os} STREQUAL "Linux")
  set(DYNINST_OS_Linux TRUE)
elseif(${_host_os} STREQUAL "FreeBSD")
  set(DYNINST_OS_FreeBSD TRUE)
elseif(${_host_os} STREQUAL "Windows")
  set(DYNINST_OS_Windows TRUE)
endif()

# The CMake `UNIX` covers more than just Linux and FreeBSD, so make
# a more limited version.
if(DYNINST_OS_Linux OR DYNINST_OS_FreeBSD)
  set(DYNINST_OS_UNIX TRUE)
endif()

if(${_host_arch} IN_LIST _64bit_x86_arches)
  set(DYNINST_HOST_ARCH_X86_64 TRUE)
elseif(${_host_arch} STREQUAL "aarch64")
  set(DYNINST_HOST_ARCH_AARCH64 TRUE)
elseif(${_host_arch} STREQUAL "ppc64le")
  set(DYNINST_HOST_ARCH_PPC64LE TRUE)
elseif(${_host_arch} STREQUAL "riscv64")
  set(DYNINST_HOST_ARCH_RISCV64 TRUE)
  set(DYNINST_ENABLE_CAPSTONE ON)
elseif(${_host_arch} IN_LIST _32bit_x86_arches)
  set(DYNINST_HOST_ARCH_I386 TRUE)
endif()

set(_codegen_arch ${_host_arch})
if(DYNINST_CODEGEN_ARCH)
  if(NOT DYNINST_CODEGEN_ARCH IN_LIST _known_arches)
    message(
      FATAL_ERROR
        "Unsupported DYNINST_CODEGEN_ARCH, expect one of `${_known_arches}`, got `${DYNINST_CODEGEN_ARCH}`"
      )
  else()
    set(${_codegen_arch} ${DYNINST_CODEGEN_ARCH})
  endif()
endif()

if(${_codegen_arch} IN_LIST _64bit_x86_arches)
  set(DYNINST_CODEGEN_ARCH_X86_64 TRUE)
elseif(${_codegen_arch} STREQUAL "aarch64")
  set(DYNINST_CODEGEN_ARCH_AARCH64 TRUE)
elseif(${_codegen_arch} STREQUAL "riscv64")
  set(DYNINST_CODEGEN_ARCH_RISCV64 TRUE)
elseif(${_codegen_arch} STREQUAL "ppc64le")
  set(DYNINST_CODEGEN_ARCH_PPC64LE TRUE)
elseif(${_codegen_arch} STREQUAL "riscv64")
  set(DYNINST_CODEGEN_ARCH_RISCV64 TRUE)
  set(DYNINST_ENABLE_CAPSTONE ON)
elseif(${_codegen_arch} IN_LIST _32bit_x86_arches)
  set(DYNINST_CODEGEN_ARCH_I386 TRUE)
elseif(${_codegen_arch} STREQUAL "amdgpu_gfx908")
  set(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908 TRUE)
elseif(${_codegen_arch} STREQUAL "amdgpu_gfx90a")
  set(DYNINST_CODEGEN_ARCH_AMDGPU_GFX90A TRUE)
elseif(${_codegen_arch} STREQUAL "amdgpu_gfx940")
  set(DYNINST_CODEGEN_ARCH_AMDGPU_GFX940 TRUE)
else()
  message(FATAL_ERROR "Missing CODEGEN ARCH")
endif()

# --- DEPRECATED ---
# For legacy support in the testsuite ONLY
if(${_host_os} STREQUAL "Linux")
  if(NOT _is64bit)
    set(DYNINST_PLATFORM i386-unknown-linux2.4)
  else()
    if(${_host_arch} IN_LIST _64bit_x86_arches)
      set(DYNINST_PLATFORM x86_64-unknown-linux2.4)
    elseif(${_host_arch} STREQUAL "aarch64")
      set(DYNINST_PLATFORM aarch64-unknown-linux)
    else()
      set(DYNINST_PLATFORM ppc64_linux)
    endif()
  endif()
elseif(${_host_os} STREQUAL "FreeBSD")
  if(_is64bit)
    set(DYNINST_PLATFORM amd64-unknown-freebsd7.2)
  else()
    set(DYNINST_PLATFORM i386-unknown-freebsd7.2)
  endif()
elseif(${_host_os} STREQUAL "Windows")
  set(DYNINST_PLATFORM i386-unknown-nt4.0)
endif()
