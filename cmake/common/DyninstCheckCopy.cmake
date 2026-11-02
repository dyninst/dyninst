# Checks that a repository's copy of cmake/common matches Dyninst's.  Each
# repository that copies the directory runs this in its own CI, so a copy that
# was edited directly, or not updated after a change in Dyninst, is reported.
#
# Run with 'cmake -P', expecting:
#
#   COPY    the copy, such as cmake/common
#   MASTER  Dyninst's cmake/common, from a checkout of Dyninst's master
#
# A relative path is taken from the directory cmake is run in.
#
# It exits 0 when the two hold the same files with the same contents, and 1
# otherwise, after listing the files that differ, are missing or are extra.

# For IN_LIST, which a script run with 'cmake -P' gets only from a minimum
cmake_minimum_required(VERSION 3.14)

# A section is its title between two rules, two blank lines after the previous
# one.  Everything is plain message(), on stderr.
set(_section_printed FALSE)
set(_section_rule "--------------------------------------------------")

macro(dyninst_check_copy_section _title)
  if(_section_printed)
    message("\n")
  endif()
  message("${_section_rule}\n-- ${_title}\n${_section_rule}")
  set(_section_printed TRUE)
endmacro()

# A relative path is taken from the current directory
foreach(_var COPY MASTER)
  get_filename_component(
    ${_var}
    "${${_var}}"
    ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  if(NOT IS_DIRECTORY "${${_var}}")
    message(FATAL_ERROR "DyninstCheckCopy: ${_var} '${${_var}}' is not a directory")
  endif()
endforeach()

file(GLOB_RECURSE _copy_files LIST_DIRECTORIES false RELATIVE "${COPY}" "${COPY}/*")
file(GLOB_RECURSE _master_files LIST_DIRECTORIES false RELATIVE "${MASTER}" "${MASTER}/*")
list(SORT _copy_files)
list(SORT _master_files)

set(_differ "")
set(_missing "")
foreach(_f IN LISTS _master_files)
  if(NOT _f IN_LIST _copy_files)
    list(APPEND _missing "${_f}")
  else()
    file(SHA256 "${COPY}/${_f}" _copy_hash)
    file(SHA256 "${MASTER}/${_f}" _master_hash)
    if(NOT _copy_hash STREQUAL _master_hash)
      list(APPEND _differ "${_f}")
    endif()
  endif()
endforeach()
set(_extra "")
foreach(_f IN LISTS _copy_files)
  if(NOT _f IN_LIST _master_files)
    list(APPEND _extra "${_f}")
  endif()
endforeach()

if(NOT _differ AND NOT _missing AND NOT _extra)
  list(LENGTH _master_files _count)
  dyninst_check_copy_section("cmake/common: all ${_count} files match Dyninst's")
  return()
endif()

foreach(_kind differ missing extra)
  if(_${_kind})
    if(_kind STREQUAL "differ")
      dyninst_check_copy_section("files that differ from Dyninst's")
    elseif(_kind STREQUAL "missing")
      dyninst_check_copy_section("files Dyninst has and the copy does not")
    else()
      dyninst_check_copy_section("files the copy has and Dyninst does not")
    endif()
    string(REPLACE ";" "\n  " _list "${_${_kind}}")
    message("  ${_list}")
  endif()
endforeach()

dyninst_check_copy_section("to fix")
message(
  "cmake/common is changed only in Dyninst and copied from there.  Make the\n"
  "change in Dyninst first, then replace this copy with Dyninst's:\n"
  "\n"
  "    rm -r cmake/common\n"
  "    cp -r <dyninst>/cmake/common cmake/common\n"
)
message(FATAL_ERROR "cmake/common does not match Dyninst's")
