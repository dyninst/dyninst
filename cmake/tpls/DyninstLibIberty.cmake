#================================================================
#
# Configure libiberty
#
#   ----------------------------------------
#
# LibIberty_ROOT_DIR - Directory hint for libiberty installation
#
#================================================================

include_guard(GLOBAL)

# libiberty is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX)
    if(NOT TARGET Dyninst::LibIberty)
        add_library(Dyninst::LibIberty INTERFACE)
    endif()
    return()
endif()

# Base directory the of LibIberty installation
set(LibIberty_ROOT_DIR
    "/usr"
    CACHE PATH "Base directory the of LibIberty installation")
mark_as_advanced(LibIberty_ROOT_DIR)

find_package(LibIberty REQUIRED)

if(NOT TARGET Dyninst::LibIberty)
	add_library(Dyninst::LibIberty INTERFACE IMPORTED)
	target_include_directories(Dyninst::LibIberty SYSTEM INTERFACE LibIberty::LibIberty)
	target_link_libraries(Dyninst::LibIberty INTERFACE LibIberty::LibIberty)
endif()
