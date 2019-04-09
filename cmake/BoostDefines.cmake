######################################################
#   BoostDefines.cmake
#
#   Compiler defines for Boost
#
#   These are separate so that they can be used
#   in external projects without needing to
#   re-find the Boost libraries.
#
######################################################

# Disable auto-linking
add_definitions(-DBOOST_ALL_NO_LIB=1)

# Disable generating serialization code in boost::multi_index
add_definitions(-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

# There are broken versions of MSVC that won't handle variadic templates
# correctly (despite the C++11 test case passing).
if(MSVC)
  add_definitions(-DBOOST_NO_CXX11_VARIADIC_TEMPLATES)
endif()
