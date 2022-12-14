#================================================================
#
# Configure valgrind
#
#   ----------------------------------------
#
# Valgrind_ROOT_DIR - Directory hint for valgrind installation
#
#================================================================

include_guard(GLOBAL)

# valgrind is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX)
    if(NOT TARGET Dyninst::Valgrind)
        add_library(Dyninst::Valgrind INTERFACE)
    endif()
    return()
endif()

# Base directory the of Valgrind installation
set(Valgrind_ROOT_DIR
    "/usr"
    CACHE PATH "Base directory the of Valgrind installation")
mark_as_advanced(Valgrind_ROOT_DIR)

find_package(Valgrind REQUIRED)

if(NOT TARGET Dyninst::Valgrind)
	add_library(Dyninst::Valgrind INTERFACE IMPORTED)
	target_include_directories(Dyninst::Valgrind SYSTEM INTERFACE ${Valgrind_INCLUDE_DIRS})
	target_link_libraries(Dyninst::Valgrind INTERFACE Valgrind::Valgrind)
endif()
