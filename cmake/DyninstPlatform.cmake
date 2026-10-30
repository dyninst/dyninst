include_guard(GLOBAL)

# Equivalent to CMAKE_HOST_SYSTEM_NAME and `uname -s` on Unixes
cmake_host_system_information(RESULT _host_os QUERY OS_NAME)

set(_known_oses "Linux" "FreeBSD" "Windows")
if(NOT "${_host_os}" IN_LIST _known_oses)
  message(FATAL_ERROR "Unsupported OS: '${_host_os}'")
endif()

# The architectures Dyninst can be built on, and the ones it can generate code for,
# by the names DYNINST_HOST_ARCH and DYNINST_CODEGEN_ARCH use. Adding an
# architecture means adding it here, and any other name for it below.
set(
  DYNINST_HOST_ARCHES
  "x86_64"
  "i386"
  "aarch64"
  "ppc64le"
  "riscv64"
)
set(
  DYNINST_CODEGEN_ARCHES
  ${DYNINST_HOST_ARCHES}
  "amdgpu_gfx908"
  "amdgpu_gfx90a"
  "amdgpu_gfx940"
  "amdgpu_gfx950"
)

# Also supported are aliases for an architecture, as `uname -m` or DYNINST_CODEGEN_ARCH
# may give it.
# For each <alias> of a canonical <arch>, set _dyninst_arch_alias_<alias> to <arch>.
set(_dyninst_arch_alias_amd64 "x86_64")
set(_dyninst_arch_alias_i686 "i386")

# Sets <out> to the name of architecture <arch> in <valid>, resolving aliases, and
# DYNINST_<kind>_ARCH_<ARCH> to TRUE in the caller. Stops with <what> in the message
# if <arch> is not in <valid>.
function(
  _dyninst_set_arch
  out
  kind
  arch
  valid
  what
)
  if(DEFINED _dyninst_arch_alias_${arch})
    set(arch ${_dyninst_arch_alias_${arch}})
  endif()
  if(NOT "${arch}" IN_LIST ${valid})
    message(FATAL_ERROR "Unsupported ${what}: '${ARGV2}', expected one of ${${valid}}")
  endif()
  string(TOUPPER ${arch} _upper)
  set(${out} ${arch} PARENT_SCOPE)
  set(DYNINST_${kind}_ARCH_${_upper} TRUE PARENT_SCOPE)
endfunction()

# Equivalent to CMAKE_HOST_SYSTEM_PROCESSOR and `uname -m` on Unixes
cmake_host_system_information(RESULT _host_arch QUERY OS_PLATFORM)

# DYNINST_HOST_ARCH and DYNINST_CODEGEN_ARCH name the two architectures as the
# DYNINST_*_ARCH_* variables do, in lower case: x86_64 for amd64, i386 for i686.
# DYNINST_CODEGEN_ARCH, set below, hides the option of that name: it is the
# architecture the option selects, the host's when the option is empty.
_dyninst_set_arch(
  DYNINST_HOST_ARCH
  HOST
  "${_host_arch}"
  DYNINST_HOST_ARCHES
  "architecture"
)

# Equivalent to checking CMAKE_SIZEOF_VOID_P
cmake_host_system_information(RESULT _is64bit QUERY IS_64BIT)

# 32-bit is only supported on 32-bit x86
if(NOT _is64bit AND NOT DYNINST_HOST_ARCH_I386)
  message(FATAL_ERROR "32-bit programming is only supported on i386")
endif()

# These checks are redundant, but protect against string name changes
if("${_host_os}" STREQUAL "Linux")
  set(DYNINST_OS_Linux TRUE)
elseif("${_host_os}" STREQUAL "FreeBSD")
  set(DYNINST_OS_FreeBSD TRUE)
elseif("${_host_os}" STREQUAL "Windows")
  set(DYNINST_OS_Windows TRUE)
endif()

# The CMake `UNIX` covers more than just Linux and FreeBSD, so make
# a more limited version.
if(DYNINST_OS_Linux OR DYNINST_OS_FreeBSD)
  set(DYNINST_OS_UNIX TRUE)
endif()

set(_codegen_arch ${DYNINST_HOST_ARCH})
if(DYNINST_CODEGEN_ARCH)
  set(_codegen_arch ${DYNINST_CODEGEN_ARCH})
endif()
_dyninst_set_arch(
  DYNINST_CODEGEN_ARCH
  CODEGEN
  "${_codegen_arch}"
  DYNINST_CODEGEN_ARCHES
  "DYNINST_CODEGEN_ARCH"
)

# Capstone decodes riscv64
if(DYNINST_HOST_ARCH_RISCV64 OR DYNINST_CODEGEN_ARCH_RISCV64)
  set(DYNINST_ENABLE_CAPSTONE ON)
endif()

# Whether Dyninst generates code for the host it is built on, so that a
# program it instruments or rewrites can run here
if("${DYNINST_HOST_ARCH}" STREQUAL "${DYNINST_CODEGEN_ARCH}")
  set(DYNINST_CODEGEN_NATIVE TRUE)
else()
  set(DYNINST_CODEGEN_NATIVE FALSE)
endif()

# --- DEPRECATED ---
# For legacy support in the testsuite ONLY
if("${_host_os}" STREQUAL "Linux")
  if(NOT _is64bit)
    set(DYNINST_PLATFORM i386-unknown-linux2.4)
  else()
    if(DYNINST_HOST_ARCH_X86_64)
      set(DYNINST_PLATFORM x86_64-unknown-linux2.4)
    elseif(DYNINST_HOST_ARCH_AARCH64)
      set(DYNINST_PLATFORM aarch64-unknown-linux)
    else()
      set(DYNINST_PLATFORM ppc64_linux)
    endif()
  endif()
elseif("${_host_os}" STREQUAL "FreeBSD")
  if(_is64bit)
    set(DYNINST_PLATFORM amd64-unknown-freebsd7.2)
  else()
    set(DYNINST_PLATFORM i386-unknown-freebsd7.2)
  endif()
elseif("${_host_os}" STREQUAL "Windows")
  set(DYNINST_PLATFORM i386-unknown-nt4.0)
endif()

message(STATUS "DYNINST_HOST_ARCH = ${DYNINST_HOST_ARCH}")
message(STATUS "DYNINST_CODEGEN_ARCH = ${DYNINST_CODEGEN_ARCH}")
message(STATUS "DYNINST_CODEGEN_NATIVE = ${DYNINST_CODEGEN_NATIVE}")
message(STATUS "DYNINST_PLATFORM = ${DYNINST_PLATFORM}")
