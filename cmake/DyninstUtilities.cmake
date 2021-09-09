# include guard
include_guard(DIRECTORY)

# Utilities - useful macros and functions for generic tasks
#

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW)

include(CMakeDependentOption)
include(CMakeParseArguments)

# generic options starting can conflict when dyninst is submodule
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(DYNINST_OPTION_PREFIX "Option names are prefixed with 'DYNINST_'" ON)
    option(DYNINST_QUIET_CONFIG "Suppress configuration messages" ON)
else()
    option(DYNINST_OPTION_PREFIX "Option names are prefixed with 'DYNINST_'" OFF)
    option(DYNINST_QUIET_CONFIG "Suppress configuration messages" OFF)
endif()

mark_as_advanced(DYNINST_OPTION_PREFIX)
mark_as_advanced(DYNINST_QUIET_CONFIG)

#-----------------------------------------------------------------------
# message which handles DYNINST_QUIET_CONFIG settings
#-----------------------------------------------------------------------
#
FUNCTION(DYNINST_MESSAGE TYPE)
    if("${TYPE}" STREQUAL "FATAL_ERROR")
        message(${TYPE} "[dyninst] ${ARGN}")
    else()
        if(NOT DYNINST_QUIET_CONFIG)
            message(${TYPE} "[dyninst] ${ARGN}")
        endif()
    endif()
ENDFUNCTION()


#----------------------------------------------------------------------------------------#
# call before running check_{c,cxx}_compiler_flag
#
macro(DYNINST_BEGIN_FLAG_CHECK)
    if(DYNINST_QUIET_CONFIG)
        if(NOT DEFINED CMAKE_REQUIRED_QUIET)
            set(CMAKE_REQUIRED_QUIET OFF)
        endif()
        set(_DYNINST_FLAG_CHECK_CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET})
        set(CMAKE_REQUIRED_QUIET ON)
    endif()
endmacro()


#----------------------------------------------------------------------------------------#
# call after running check_{c,cxx}_compiler_flag
#
macro(DYNINST_END_FLAG_CHECK)
    if(DYNINST_QUIET_CONFIG)
        set(CMAKE_REQUIRED_QUIET ${_DYNINST_FLAG_CHECK_CMAKE_REQUIRED_QUIET})
        unset(_DYNINST_FLAG_CHECK_CMAKE_REQUIRED_QUIET)
    endif()
endmacro()


#-----------------------------------------------------------------------
# Apply operation to a global property list
#
FUNCTION(DYNINST_APPEND_PROPERTY_LIST _PROP)
    get_property(_ENTRY GLOBAL PROPERTY ${_PROP})
    set(_ARGS ${ARGN})
    if(_ARGS)
        list(REMOVE_DUPLICATES _ARGS)
    endif()
    foreach(_ARG ${_ARGS})
        if(NOT ${_ARG} IN_LIST _ENTRY)
            set_property(GLOBAL APPEND PROPERTY ${_PROP} ${_ARG})
        endif()
    endforeach()
ENDFUNCTION()


#-----------------------------------------------------------------------
# function dyninst_get_property(<NAME> <NAMES...>)
#          Defines a local variable with the value of the global property, e.g.
#
#               dyninst_get_property(Dyninst_ENABLED_INTERFACES)
#               message(STATUS "Dyninst enabled interfaces: ${Dyninst_ENABLE_INTERFACES}")
#
FUNCTION(DYNINST_GET_PROPERTY _prop)
    set(_ENTRIES)
    foreach(_ARG ${_prop} ${ARGN})
        get_property(_ENTRY GLOBAL PROPERTY ${_ARG})
        list(APPEND _ENTRIES ${_ENTRY})
    endforeach()
    set(${_prop} ${_ENTRIES} PARENT_SCOPE)
ENDFUNCTION()


#----------------------------------------------------------------------------------------#
# macro to add an interface lib
#
FUNCTION(DYNINST_ADD_INTERFACE_LIBRARY _TARGET)
    # add the build-tree target
    add_library(${_TARGET} INTERFACE)
    # create an alias which is identical to the one in the install tree
    add_library(${PROJECT_NAME}::${_TARGET} ALIAS ${_TARGET})
    # add this to the dyninst targets so the export will get read in DyninstConfig.cmake
    dyninst_append_property_list(ALL_DYNINST_TARGETS ${_TARGET})
    # make sure this is defined
    if(NOT DEFINED INSTALL_CMAKE_DIR)
        set(INSTALL_CMAKE_DIR lib/cmake/${PROJECT_NAME})
    endif()
    # installation
    install(TARGETS ${_TARGET}
        EXPORT ${_TARGET}Targets
        COMPONENT ${_TARGET})
    install(EXPORT ${_TARGET}Targets
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_CMAKE_DIR})
    # build tree
    export(TARGETS  ${_TARGET}
        NAMESPACE   ${PROJECT_NAME}::
        FILE        ${PROJECT_BINARY_DIR}/build-tree/${_TARGET}Targets.cmake
        EXPORT_LINK_INTERFACE_LIBRARIES)
ENDFUNCTION()


#-----------------------------------------------------------------------
# function dyninst_add_feature(<NAME> <DOCSTRING>)
#          Add a project feature, whose activation is specified by the
#          existence of the variable <NAME>, to the list of enabled/disabled
#          features, plus a docstring describing the feature
#
FUNCTION(DYNINST_ADD_FEATURE _var _description)
  set(EXTRA_DESC "")
  foreach(currentArg ${ARGN})
      if(NOT "${currentArg}" STREQUAL "${_var}" AND
         NOT "${currentArg}" STREQUAL "${_description}" AND
         NOT "${currentArg}" STREQUAL "CMAKE_DEFINE" AND
         NOT "${currentArg}" STREQUAL "DOC")
          set(EXTRA_DESC "${EXTA_DESC}${currentArg}")
      endif()
  endforeach()

  set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_FEATURES ${_var})
  set_property(GLOBAL PROPERTY ${_var}_DESCRIPTION "${_description}${EXTRA_DESC}")
ENDFUNCTION()


#----------------------------------------------------------------------------------------#
# function add_option(<OPTION_NAME> <DOCSRING> <DEFAULT_SETTING> [NO_FEATURE])
#          Add an option and add as a feature if NO_FEATURE is not provided
#
FUNCTION(DYNINST_ADD_OPTION _NAME _MESSAGE _DEFAULT)
    if(DYNINST_OPTION_PREFIX)
        option(DYNINST_${_NAME} "${_MESSAGE}" ${_DEFAULT})
        # set the option locally for this project
        set(${_NAME} ${DYNINST_${_NAME}} PARENT_SCOPE)
        # for after this if/else
        set(_NAME DYNINST_${_NAME})
    else()
        option(${_NAME} "${_MESSAGE}" ${_DEFAULT})
        # set the option locally for this project
        set(DYNINST_${_NAME} ${${_NAME}} PARENT_SCOPE)
    endif()
    # mark as advanced or add as feature
    if("ADVANCED" IN_LIST ARGN)
        mark_as_advanced(${_NAME})
    else()
        dyninst_add_feature(${_NAME} "${_MESSAGE}")
    endif()
ENDFUNCTION()


FUNCTION(DYNINST_FORCE_OPTION _NAME _VALUE)
    # updated non DYNINST_ prefixed version if in cache
    if(NOT "$CACHE{${_NAME}}" STREQUAL "")
        get_property(_TYPE CACHE ${_NAME} PROPERTY TYPE)
        get_property(_DESC CACHE ${_NAME} PROPERTY HELPSTRING)
        set(${_NAME} ${_VALUE} CACHE ${_TYPE} "${_DESC}" FORCE)
    endif()
    # update DYNINST_ prefixed version if in cache
    if(NOT "$CACHE{DYNINST_${_NAME}}" STREQUAL "")
        get_property(_TYPE CACHE DYNINST_${_NAME} PROPERTY TYPE)
        get_property(_DESC CACHE DYNINST_${_NAME} PROPERTY HELPSTRING)
        set(DYNINST_${_NAME} ${_VALUE} CACHE ${_TYPE} "${_DESC}" FORCE)
    endif()
    # ensure the local variable is updated
    if(DEFINED ${_NAME})
        set(${_NAME} ${_VALUE})
    endif()
    if(DEFINED DYNINST_${_NAME})
        set(DYNINST_${_NAME} ${_VALUE})
    endif()
ENDFUNCTION()

#----------------------------------------------------------------------------------------#
# function print_enabled_features()
#          Print enabled  features plus their docstrings.
#
FUNCTION(DYNINST_PRINT_ENABLED_FEATURES)
    FUNCTION(DYNINST_CAPITALIZE str var)
        # make string lower
        string(TOLOWER "${str}" str)
        string(SUBSTRING "${str}" 0 1 _first)
        string(TOUPPER "${_first}" _first)
        string(SUBSTRING "${str}" 1 -1 _remainder)
        string(CONCAT str "${_first}" "${_remainder}")
        set(${var} "${str}" PARENT_SCOPE)
    ENDFUNCTION()
    set(_basemsg "The following features are defined/enabled (+):")
    set(_currentFeatureText "${_basemsg}")
    get_property(_features GLOBAL PROPERTY ${PROJECT_NAME}_FEATURES)
    if(NOT "${_features}" STREQUAL "")
        list(REMOVE_DUPLICATES _features)
        list(SORT _features)
    endif()
    foreach(_feature ${_features})
        if(${_feature})
            # add feature to text
            set(_currentFeatureText "${_currentFeatureText}\n     ${_feature}")
            # get description
            get_property(_desc GLOBAL PROPERTY ${_feature}_DESCRIPTION)
            # print description, if not standard ON/OFF, print what is set to
            if(_desc)
                if(NOT "${${_feature}}" STREQUAL "ON" AND
                   NOT "${${_feature}}" STREQUAL "TRUE")
                    set(_currentFeatureText "${_currentFeatureText}: ${_desc} -- [\"${${_feature}}\"]")
                else()
                    string(REGEX REPLACE "^${PROJECT_NAME}_USE_" "" _feature_tmp "${_feature}")
                    string(TOLOWER "${_feature_tmp}" _feature_tmp_l)
                    dyninst_capitalize("${_feature_tmp}" _feature_tmp_c)
                    foreach(_var _feature _feature_tmp _feature_tmp_l _feature_tmp_c)
                        set(_ver "${${${_var}}_VERSION}")
                        if(NOT "${_ver}" STREQUAL "")
                            set(_desc "${_desc} -- [found version ${_ver}]")
                            break()
                        endif()
                        unset(_ver)
                    endforeach()
                    set(_currentFeatureText "${_currentFeatureText}: ${_desc}")
                endif()
                set(_desc NOTFOUND)
            endif()
        endif()
    endforeach()

    if(NOT "${_currentFeatureText}" STREQUAL "${_basemsg}")
        dyninst_message(STATUS "${_currentFeatureText}\n")
    endif()
ENDFUNCTION()


#----------------------------------------------------------------------------------------#
# function dyninst_print_disabled_features()
#          Print disabled features plus their docstrings.
#
FUNCTION(DYNINST_PRINT_DISABLED_FEATURES)
    set(_basemsg "The following features are NOT defined/enabled (-):")
    set(_currentFeatureText "${_basemsg}")
    get_property(_features GLOBAL PROPERTY ${PROJECT_NAME}_FEATURES)
    if(NOT "${_features}" STREQUAL "")
        list(REMOVE_DUPLICATES _features)
        list(SORT _features)
    endif()
    foreach(_feature ${_features})
        if(NOT ${_feature})
            set(_currentFeatureText "${_currentFeatureText}\n     ${_feature}")

            get_property(_desc GLOBAL PROPERTY ${_feature}_DESCRIPTION)

            if(_desc)
              set(_currentFeatureText "${_currentFeatureText}: ${_desc}")
              set(_desc NOTFOUND)
            endif(_desc)
        endif()
    endforeach(_feature)

    if(NOT "${_currentFeatureText}" STREQUAL "${_basemsg}")
        dyninst_message(STATUS "${_currentFeatureText}\n")
    endif()
ENDFUNCTION()


#----------------------------------------------------------------------------------------#
# function dyninst_print_features()
#          Print all features plus their docstrings.
#
FUNCTION(DYNINST_PRINT_FEATURES)
    dyninst_message(STATUS "")
    dyninst_print_enabled_features()
    dyninst_print_disabled_features()
ENDFUNCTION()


cmake_policy(POP)
