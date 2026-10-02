# Runs gersemi over this project's own CMake code.  Driven by the cmake-check,
# cmake-check-raw, cmake-reformat and cmake-reformat-diff targets; see
# DyninstCMakeFormat.cmake.
#
# Run with 'cmake -P', expecting:
#
#   GERSEMI     path to gersemi, or empty if it was not found
#   MODE        check | check-raw | diff | in-place
#   SOURCE_DIR  top of the source tree
#
# The file list is 'git ls-files' filtered the way the CMake Formatting
# workflow filters it, so the targets and CI look at exactly the same files.
# It also keeps the build tree out: an in-source build directory holds
# hundreds of generated .cmake files that a glob would happily reformat.
#
# The long messages below are message() with no mode rather than part of the
# FATAL_ERROR, because FATAL_ERROR indents what it is given and puts a blank
# line between every pair of lines, which makes a command meant to be copied
# hard to read.  Plain message() writes to stderr with the text untouched on
# every CMake this project supports; NOTICE would need 3.15.

# A section is headed by its title between two rules, and separated from the
# one before it by two blank lines, so that a file list of any length does not
# run into whatever follows it:
#
#     --------------------------------------------------
#     -- gersemi --check on 95 files: reformatting required
#     --------------------------------------------------
#
# These are plain message(), not message(STATUS), for the stream as much as
# for the '-- ' that STATUS would prefix: STATUS writes to stdout, and
# everything else here -- the file list, the warnings, gersemi's own report --
# is on stderr.  Mixing the two splits a section from its heading the moment
# either stream is redirected.  Keeping the headings on stderr also leaves
# stdout carrying nothing this script wrote, so in diff mode it is gersemi's
# diff and nothing else.
set(_section_printed FALSE)

# Spelled out rather than built with string(REPEAT), which needs CMake 3.15;
# this project supports 3.14.
set(_section_rule "--------------------------------------------------")

macro(dyninst_gersemi_section _title)
  if(_section_printed)
    message("\n")
  endif()
  string(REPLACE "\n" "\n-- " _heading "${_title}")
  message("${_section_rule}\n-- ${_heading}\n${_section_rule}")
  set(_section_printed TRUE)
endmacro()

if(NOT GERSEMI OR GERSEMI MATCHES "NOTFOUND$")
  dyninst_gersemi_section("installing gersemi")
  message(
    "The cmake-check, cmake-check-raw, cmake-reformat and\n"
    "cmake-reformat-diff targets format this project's CMake code with\n"
    "gersemi.  Install it with\n"
    "\n"
    "    python3 -m pip install gersemi\n"
    "\n"
    "and re-run cmake, or name it directly with\n"
    "\n"
    "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<path> <build-dir>\n"
  )
  message(FATAL_ERROR "gersemi was not found")
endif()

find_package(Git QUIET)
if(NOT Git_FOUND)
  dyninst_gersemi_section("installing git")
  message(
    "The cmake-* formatting targets ask git which files to format, so that\n"
    "they and the CMake Formatting workflow always agree on the list.\n"
  )
  message(FATAL_ERROR "git was not found")
endif()

execute_process(
  COMMAND "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" ls-files
  OUTPUT_VARIABLE _tracked
  ERROR_VARIABLE _git_error
  RESULT_VARIABLE _git_result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT _git_result EQUAL 0)
  message(FATAL_ERROR "'git ls-files' failed in ${SOURCE_DIR}: ${_git_error}")
endif()

string(REPLACE "\n" ";" _files "${_tracked}")
list(FILTER _files INCLUDE REGEX "(^|/)CMakeLists\\.txt$|\\.cmake$")

# git lists the index; gersemi reads the working tree.  A file deleted with
# plain rm is still in the index, and handing gersemi a path that is not there
# makes it die with a Python traceback -- which this script would then file
# under "other warnings" and report as a formatting failure.  Drop them.
set(_present "")
foreach(_f IN LISTS _files)
  if(EXISTS "${SOURCE_DIR}/${_f}")
    list(APPEND _present "${_f}")
  endif()
endforeach()
set(_files "${_present}")

list(LENGTH _files _count)
if(_count EQUAL 0)
  message(FATAL_ERROR "no CMake files are tracked under ${SOURCE_DIR}")
endif()
if(MODE STREQUAL "check" OR MODE STREQUAL "check-raw")
  set(_flags --check)
elseif(MODE STREQUAL "diff")
  # --color colours the diff through colorama, which strips its own output
  # when stdout is not a terminal -- and execute_process always hands the
  # child a pipe, so today this changes nothing either way.  It costs
  # nothing, and it is what makes the diff colour the day that stops being
  # true.
  #
  # Unknown-command warnings belong to cmake-check, which has a section for
  # them.  Here they are someone else's subject interleaved with the diff
  # that was asked for, and leaving them out does not change a byte of it.
  set(_flags --diff --color --no-warn-about-unknown-commands)
elseif(MODE STREQUAL "in-place")
  set(_flags --in-place)
else()
  message(FATAL_ERROR "DyninstRunGersemi: MODE '${MODE}' is not a mode")
endif()

# ${_flags} is a list, which interpolates into a message with semicolons
# between its elements rather than spaces.
list(JOIN _flags " " _flags_text)

if(MODE STREQUAL "check")
  execute_process(
    COMMAND "${GERSEMI}" ${_flags} ${_files}
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr
    RESULT_VARIABLE _result
  )

  # gersemi puts both kinds of line on stderr, so shape is all there is to
  # tell them apart: a file that needs work is one line ending in a fixed
  # suffix, and a warning is a free-form block of however many lines.
  # Dropping the suffix and the source directory leaves the part that
  # carries the information.
  set(_text "${_stdout}${_stderr}")
  string(REPLACE "${SOURCE_DIR}/" "" _text "${_text}")
  string(REPLACE "\n" ";" _lines "${_text}")

  set(_reformat "")
  set(_warnings "")
  foreach(_line IN LISTS _lines)
    if(_line MATCHES "^(.+) would be reformatted$")
      list(APPEND _reformat "${CMAKE_MATCH_1}")
    else()
      string(APPEND _warnings "${_line}\n")
    endif()
  endforeach()
  string(STRIP "${_warnings}" _warnings)
  list(LENGTH _reformat _reformat_count)

  # gersemi hands its files to a pool of workers, so the order it reports
  # them in changes from run to run.  Sorting makes two runs comparable.
  if(_reformat_count GREATER 0)
    list(SORT _reformat)
    list(JOIN _reformat "\n  " _joined)
    dyninst_gersemi_section("gersemi --check on ${_count} files: reformatting required")
    message("  ${_joined}")
  endif()

  if(NOT _warnings STREQUAL "")
    dyninst_gersemi_section("gersemi --check on ${_count} files: other warnings")
    message("${_warnings}")
  endif()

  if(_reformat_count EQUAL 0 AND _warnings STREQUAL "")
    dyninst_gersemi_section("gersemi --check on ${_count} files: all formatted")
  endif()
else()
  dyninst_gersemi_section("gersemi ${_flags_text} on ${_count} files")
  execute_process(
    COMMAND "${GERSEMI}" ${_flags} ${_files}
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _result
  )
endif()

if(NOT _result EQUAL 0)
  if(MODE MATCHES "^check")
    dyninst_gersemi_section("reformatting and inspecting")
    message(
      "The files listed above are not formatted.  To see what would change:\n"
      "\n"
      "    cmake --build <build-dir> --target cmake-reformat-diff\n"
      "\n"
      "To make the change:\n"
      "\n"
      "    cmake --build <build-dir> --target cmake-reformat\n"
    )
    message(FATAL_ERROR "the CMake code is not formatted")
  else()
    message(FATAL_ERROR "gersemi ${_flags_text} exited with ${_result}")
  endif()
endif()
