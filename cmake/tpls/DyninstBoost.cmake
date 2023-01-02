#===========================================================
#
# Configure Boost
#
#   ----------------------------------------
#
# Boost_ROOT_DIR - Directory hint for Boost installation
#
#===========================================================

include_guard(GLOBAL)

# Need at least 1.71 for a usable BoostConfig.cmake
set(_min_version 1.71.0)

# Use multithreaded libraries
set(Boost_USE_MULTITHREADED ON)

# Don't use libraries linked statically to the C++ runtime
set(Boost_USE_STATIC_RUNTIME OFF)

# Set the default location to look for Boost
set(Boost_ROOT_DIR
    "/usr"
    CACHE PATH "Boost root directory for Dyninst")
mark_as_advanced(Boost_ROOT_DIR)

# Starting in CMake 3.20, suppress "unknown version" warnings
set(Boost_NO_WARN_NEW_VERSIONS ON)

# Library components that need to be linked against
set(_boost_components atomic chrono date_time filesystem thread timer)
find_package(
    Boost
    ${_min_version}
    QUIET
    REQUIRED
    HINTS
    ${Boost_ROOT_DIR}
    ${PATH_BOOST}
    ${BOOST_ROOT}
    COMPONENTS ${_boost_components})

if(NOT TARGET Dyninst::Boost)
	# Make an interface dummy target to force includes to be treated as SYSTEM
	add_library(Dyninst::Boost INTERFACE IMPORTED)
	target_link_libraries(Dyninst::Boost INTERFACE ${Boost_LIBRARIES})
	target_include_directories(Dyninst::Boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
	target_compile_definitions(Dyninst::Boost
	                           INTERFACE BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
	
	# Just the headers (effectively a simplified Boost::headers target)
	add_library(Dyninst::Boost_headers INTERFACE IMPORTED)
	target_include_directories(Dyninst::Boost_headers SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
	target_compile_definitions(Dyninst::Boost_headers
	                           INTERFACE BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
endif()
message(STATUS "Found Boost ${Boost_VERSION}")
message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")