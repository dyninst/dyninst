# Frame sizes are larger for debug build, so adjust based on build type
# files with functions containing large frames are adjust below
# (the value could be made significantly maller if more files are adjusted).
#
set(defaultDebugMaxFrameSize 24576)
set(defaultNonDebugMaxFrameSize 20480)


# REQUESTED_WARNING_FLAGS is a list of warning flags for C and C++ programs
# to enable if supported by the compiler.  The values do not include the
# the initial '-'

list(APPEND REQUESTED_WARNING_FLAGS
        Wall
        Wextra
        Wpedantic

        Walloca
        Wcast-align
        Wcast-qual
        Wcomma-subscript
        Wctor-dtor-privacy
        Wdeprecated-copy-dtor
        Wdouble-promotion
        Wduplicated-branches
        Wduplicated-cond
        Wenum-conversion
        Wextra-semi
        Wfloat-equal
        Wformat-overflow=2
        Wformat-signedness
        Wformat=2
        Wframe-larger-than=${defaultNonDebugMaxFrameSize}
        Wjump-misses-init
        Wlogical-op
        Wmismatched-tags
        Wmissing-braces
        Wmultichar
        Wnoexcept
        Wnon-virtual-dtor
        Woverloaded-virtual
        Wpointer-arith
        Wrange-loop-construct
        Wrestrict
        Wshadow
        Wstrict-null-sentinel
        Wsuggest-attribute=format
        Wsuggest-attribute=malloc
        Wuninitialized
        Wvla
        Wvolatile
        Wwrite-strings
    )

#list(APPEND REQUESTED_WARNING_FLAGS Werror)

#list(APPEND REQUESTED_WARNING_FLAGS Wredundant-tags)
#list(APPEND REQUESTED_WARNING_FLAGS Wnull-dereference)
#list(APPEND REQUESTED_WARNING_FLAGS Wconversion)
#list(APPEND REQUESTED_WARNING_FLAGS Wzero-as-null-pointer-constant)
#list(APPEND REQUESTED_WARNING_FLAGS Wuseless-cast)
#list(APPEND REQUESTED_WARNING_FLAGS Wsuggest-override)
#list(APPEND REQUESTED_WARNING_FLAGS Wsuggest-final-types)
#list(APPEND REQUESTED_WARNING_FLAGS Wsuggest-final-methods)
#list(APPEND REQUESTED_WARNING_FLAGS Wsign-promo)
#list(APPEND REQUESTED_WARNING_FLAGS Wold-style-cast)
#list(APPEND REQUESTED_WARNING_FLAGS Walloc-zero)

if (CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
  include(CheckCCompilerFlag)
  foreach (f IN LISTS REQUESTED_WARNING_FLAGS)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_C_FLAG_${f}")
    set(CMAKE_REQUIRED_FLAGS "-${f}")
    check_c_source_compiles("int main(){return 0;}" "${v}" FAIL_REGEX "warning: *command[- ]line option|-Wunknown-warning-option")
    # Previous two lines are equivalent to below, but also catches
    # a 0 exit status with a warning message output:
    #    check_c_compiler_flag("-${f}" "${v}")
    if (${v})
        string(APPEND SUPPORTED_C_WARNING_FLAGS " -${f}")
        if (f MATCHES "^(.*)=[0-9]+$")
            # set generic variable if warning is parameterized with a number
            string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_C_FLAG_${CMAKE_MATCH_1}")
            set("${v}" 1)
        endif()
    endif()
  endforeach()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  include(CheckCXXCompilerFlag)
  foreach (f IN LISTS REQUESTED_WARNING_FLAGS)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_CPP_FLAG_${f}")
    set(CMAKE_REQUIRED_FLAGS "-${f}")
    check_cxx_source_compiles("int main(){return 0;}" "${v}" FAIL_REGEX "warning: *command[- ]line option|-Wunknown-warning-option")
    if (${v})
        string(APPEND SUPPORTED_CXX_WARNING_FLAGS " -${f}")
        if (f MATCHES "^(.*)=[0-9]+$")
            string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_CPP_FLAG_${CMAKE_MATCH_1}")
            set("${v}" 1)
        endif()
    endif()
  endforeach()
endif()

# If -Wframe-larger-than is available adjust the value to allow for larger
# frames based on compiler version and build type for the following two files:
#
#       instructionAPI/src/InstructionDecoder-power.C
#         (includes instructionAPI/src/InstructionDecoder-power.C)
#       common/src/MachSyscall.C
#         (includes common/src/SyscallInformation.C)
#
if (HAS_CPP_FLAG_Wframe_larger_than)
    # Override the default frame size maximum for DEBUG (-O0) build types
    # as there stack frames are larger:
    #
    add_compile_options($<$<CONFIG:DEBUG>:-Wframe-larger-than=${defaultDebugMaxFrameSize}>)

    # Use worst-case values discovered so far.  For most gcc versions
    # SyscallInformation.C stack frame size is less than than the default and
    # InstructionDecoder-power.C is less than 76800, but for some environments
    # and compiler configurations the following are needed:
    #
    set(debugMaxFrameSizeOverrideSyscallInformation 81920)
    set(debugMaxFrameSizeOverridePowerOpcodeTable 358400)
    if (${CMAKE_CXX_COMPILER_VERSION} MATCHES "^[7](\.|$)")
	set(nonDebugMaxFrameSizeOverridePowerOpcodeTable 38912)
    endif()
endif()

unset(CMAKE_REQUIRED_FLAGS)

if (MSVC)
  message(STATUS "TODO: Set up custom warning flags for MSVC")
  string(APPEND CMAKE_C_FLAGS "/wd4251 /wd4091 /wd4503")
  string(APPEND CMAKE_CXX_FLAGS "/wd4251 /wd4091 /wd4503")
endif()

message(STATUS "Using C warning flags: ${SUPPORTED_C_WARNING_FLAGS}")
message(STATUS "Using CXX warning flags: ${SUPPORTED_CXX_WARNING_FLAGS}")
message(STATUS "Extra CXX DEBUG warning flags: -Wframe-larger-than=${defaultDebugMaxFrameSize}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SUPPORTED_C_WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUPPORTED_CXX_WARNING_FLAGS}")
