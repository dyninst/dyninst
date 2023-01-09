include_guard(GLOBAL)

# Equivalent to CMAKE_HOST_SYSTEM_NAME and `uname -s` on Unixes
cmake_host_system_information(RESULT _host_os QUERY OS_NAME)

set(_known_oses "Linux" "FreeBSD" "Windows")
if(NOT ${_host_os} IN_LIST _known_oses)
  message(FATAL_ERROR "Unsupported OS: '${_host_os}'")
endif()

# Equivalent to CMAKE_HOST_SYSTEM_PROCESSOR and `uname -m` on Unixes
cmake_host_system_information(RESULT _host_arch QUERY OS_PLATFORM)

set(_known_arches "x86_64" "ppc64le" "aarch64" "i386" "amd64")
if(NOT ${_host_arch} IN_LIST _known_arches)
  message(FATAL_ERROR "Unsupported architecture: '${_host_arch}'")
endif()

# Equivalent to checking CMAKE_SIZEOF_VOID_P
cmake_host_system_information(RESULT _is64bit QUERY IS_64BIT)

# 32-bit is only supported on i386
if(NOT _is64bit AND NOT ${_host_arch} STREQUAL "i386")
  message(FATAL_ERROR "32-bit programming is only supported on i386")
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

if(${_host_arch} STREQUAL "x86_64" OR ${_host_arch} STREQUAL "amd64")
  set(DYNINST_ARCH_x86_64 TRUE)
elseif(${_host_arch} STREQUAL "aarch64")
  set(DYNINST_ARCH_aarch64 TRUE)
elseif(${_host_arch} STREQUAL "ppc64le")
  set(DYNINST_ARCH_ppc64le TRUE)
elseif(${_host_arch} STREQUAL "i386")
  set(DYNINST_ARCH_i386 TRUE)
endif()

# --- DEPRECATED ---
# For legacy support in the testsuite ONLY
if(${_host_os} STREQUAL "Linux")
  if(NOT _is64bit)
    set(DYNINST_PLATFORM i386-unknown-linux2.4)
  else()
    if(${_host_arch} STREQUAL "x86_64")
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
