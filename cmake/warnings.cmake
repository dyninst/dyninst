if (CMAKE_COMPILER_IS_GNUCXX)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual -Wcast-align")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wpointer-arith -Wcast-qual -Woverloaded-virtual -Wcast-align -Wno-non-template-friend")
elseif (MSVC)
# ...
endif()
