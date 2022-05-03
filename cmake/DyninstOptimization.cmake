include(DyninstUtilities)

if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif()

if(${CMAKE_C_COMPILER_ID} MATCHES Clang
   OR ${CMAKE_C_COMPILER_ID} MATCHES GNU
   OR ${CMAKE_C_COMPILER_ID} MATCHES Intel)

    if(ENABLE_LTO)
        set(LTO_LINK_FLAGS "-fuse-ld=bfd")
    else()
        set(LTO_LINK_FLAGS "")
    endif()

    set(DYNINST_C_FLAGS_DEBUG
        "-Og -g3"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_DEBUG
        "-Og -g3"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_RELEASE
        "-O2"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_RELEASE
        "-O2"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_RELWITHDEBINFO
        "-O2 -g3"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_RELWITHDEBINFO
        "-O2 -g3"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_MINSIZEREL
        "-Os"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_MINSIZEREL
        "-Os"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_MINSIZEREL
        "-Os"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_MINSIZEREL
        "-Os"
        CACHE STRING "")

    set(FORCE_FRAME_POINTER "-fno-omit-frame-pointer")

    # Ensure each library is fully linked
    set(DYNINST_SHARED_LINKER_FLAGS
        "-Wl,--no-undefined ${LTO_LINK_FLAGS}"
        CACHE STRING "")
    set(DYNINST_MODULE_LINKER_FLAGS
        "${LTO_LINK_FLAGS}"
        CACHE STRING "")
else(MSVC)
    if(ENABLE_LTO)
        set(LTO_LINK_FLAGS "/LTCG")
    else()
        set(LTO_LINK_FLAGS "")
    endif()

    set(DYNINST_C_FLAGS_DEBUG
        "/MP /Od /Zi /MDd /D_DEBUG"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_DEBUG
        "/MP /Od /Zi /MDd /D_DEBUG"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_RELEASE
        "/MP /O2 /MD"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_RELEASE
        "/MP /O2 /MD"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_RELWITHDEBINFO
        "/MP /O2 /Zi /MD"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_RELWITHDEBINFO
        "/MP /O2 /Zi /MD"
        CACHE STRING "")

    set(DYNINST_C_FLAGS_MINSIZEREL
        "/MP /O1 /MD"
        CACHE STRING "")
    set(DYNINST_CXX_FLAGS_MINSIZEREL
        "/MP /O1 /MD"
        CACHE STRING "")

    set(FORCE_FRAME_POINTER "/Oy-")

    set(DYNINST_SHARED_LINKER_FLAGS
        "${LTO_LINK_FLAGS}"
        CACHE STRING "")
    set(DYNINST_MODULE_LINKER_FLAGS
        "${LTO_LINK_FLAGS}"
        CACHE STRING "")
    set(DYNINST_STATIC_LINKER_FLAGS
        "${LTO_LINK_FLAGS}"
        CACHE STRING "")
endif()

foreach(
    _VAR
    C_FLAGS_DEBUG
    C_FLAGS_RELEASE
    C_FLAGS_MINSIZEREL
    C_FLAGS_RELWITHDEBINFO
    CXX_FLAGS_DEBUG
    CXX_FLAGS_RELEASE
    CXX_FLAGS_MINSIZEREL
    CXX_FLAGS_RELWITHDEBINFO
    SHARED_LINKER_FLAGS
    STATIC_LINKER_FLAGS
    MODULE_LINKER_FLAGS)
    if(DYNINST_${_VAR})
        dyninst_message(STATUS
                        "[CMAKE_${_VAR}] '${CMAKE_${_VAR}}' -> '${DYNINST_${_VAR}}'")
        set(CMAKE_${_VAR} ${DYNINST_${_VAR}})
    endif()
endforeach()
