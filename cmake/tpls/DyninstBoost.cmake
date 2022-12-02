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

# Need at least 1.70 because of deprecated headers
set(_boost_min_version 1.70.0)

# Use multithreaded libraries
set(Boost_USE_MULTITHREADED ON)

# Don't use libraries linked statically to the C++ runtime
set(Boost_USE_STATIC_RUNTIME OFF)

# Set the default location to look for Boost
set(Boost_ROOT_DIR
    "/usr"
    CACHE PATH "Boost root directory for Dyninst")
mark_as_advanced(Boost_ROOT_DIR)

# Library components that need to be linked against
set(_boost_components atomic chrono date_time filesystem thread timer)

find_package(Boost ${Boost_MIN_VERSION} REQUIRED HINTS ${Boost_ROOT_DIR} ${PATH_BOOST} ${BOOST_ROOT} COMPONENTS ${_boost_components})

list(TRANSFORM ${_boost_components} PREPEND "Boost::" _boost_targets)
list(APPEND _boost_targets "Boost::headers")

set(_boost_iface_dirs)
foreach(_t IN_LIST _boost_targets)
  list(APPEND _boost_iface_dirs "\$<TARGET_PROPERTY:${_t},INTERFACE_INCLUDE_DIRECTORIES>")
endforeach()

# Make an interface dummy target to force includes to be treated as SYSTEM
add_library(Dyninst::Boost INTERFACE IMPORTED)
target_link_libraries(Dyninst::Boost INTERFACE ${_boost_targets})
target_include_directories(Dyninst::Boost SYSTEM INTERFACE ${_boost_iface_dirs})
target_compile_definitions(Dyninst::Boost INTERFACE BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
