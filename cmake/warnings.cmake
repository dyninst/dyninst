if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual -Wcast-align")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual -Woverloaded-virtual -Wcast-align -Wno-non-template-friend")
elseif ("${CMAKE_C_COMPILER_ID}" MATCHES Clang)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual -Woverloaded-virtual")
elseif (MSVC)
# ...
endif()
