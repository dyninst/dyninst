# Use SymtabAPI or SymLite?
option (LIGHTWEIGHT_SYMTAB "Use lightweight symtab interface for ParseAPI, ProcControl, and Stackwalker" OFF)

# Use ParseAPI analysis in Stackwalker?
option (SW_ANALYSIS_STEPPER "Use ParseAPI-based analysis stepper in Stackwalker" ON)

#option (BUILD_RTLIB_32 "Build 32-bit runtime library on mixed 32/64 systems" ON)

# Some global on/off switches
if (LIGHTWEIGHT_SYMTAB)
add_definitions (-DWITHOUT_SYMTAB_API -DWITH_SYMLITE)
else()
add_definitions (-DWITH_SYMTAB_API -DWITHOUT_SYMLITE)
endif()

if (SW_ANALYSIS_STEPPER)
add_definitions (-DUSE_PARSE_API)
endif()

message(status "Options set")
