# CMake configuration for dynutil directory

FILE (GLOB includes "${CMAKE_CURRENT_SOURCE_DIR}/h")
INSTALL (FILES ${includes} DESTINATION include)
