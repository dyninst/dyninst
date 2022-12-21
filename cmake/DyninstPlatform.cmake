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

# i386 is only supported on Linux and Windows
if(${_host_arch} STREQUAL "i386")
	if(NOT ${_host_os} STREQUAL "Linux" AND NOT ${_host_os} STREQUAL "Windows")
		message(FATAL_ERROR "i386 is only supported on Linux and Windows")
	endif()
endif()

# FreeBSD is only supported on x86_64
if(${_host_os} STREQUAL "FreeBSD")
	if(NOT ${_host_arch} STREQUAL "x86_64" AND NOT ${_host_arch} STREQUAL "amd64") 
		message(FATAL_ERROR "FreeBSD is only supported on x86_64/amd64")
	endif()
endif()

if(${_host_os} STREQUAL "Linux")
	if(NOT _is64bit)
		set(PLATFORM i386-unknown-linux2.4)
	else()
		if(${_host_arch} STREQUAL "x86_64")
			set(PLATFORM x86_64-unknown-linux2.4)
		elseif(${_host_arch} STREQUAL "aarch64")
			set(PLATFORM aarch64-unknown-linux)
		else()
			set(PLATFORM ppc64_linux)
		endif()
	endif()
elseif(${_host_os} STREQUAL "FreeBSD")
	if(_is64bit)
  	set(PLATFORM amd64-unknown-freebsd7.2)
  else()
  	set(PLATFORM i386-unknown-freebsd7.2)
  endif()
endif()
