# Use OpenMP?
option(USE_OpenMP "Use OpenMP for parallel parsing" ON)

# Use ParseAPI analysis in Stackwalker?
option(SW_ANALYSIS_STEPPER "Use ParseAPI-based analysis stepper in Stackwalker" ON)

option(BUILD_RTLIB_32 "Build 32-bit runtime library on mixed 32/64 systems" OFF)

option(ENABLE_LTO "Enable Link-Time Optimization" OFF)

option(ENABLE_DEBUGINFOD "Enable debuginfod support" OFF)

if(SW_ANALYSIS_STEPPER)
    add_definitions(-DUSE_PARSE_API)
endif()

message(STATUS "Options set")
