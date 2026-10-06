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

# One copy of the version: docker/dependencies.versions, beside the other
# pinned dependencies.  The CI job installs what that file says, every
# install line below names the same version, and the check further down
# holds the gersemi on PATH to it -- releases format differently, so running
# cmake-reformat with another one rewrites files this tree is already correct
# for, and the job then rejects them.
set(_versions_file "${SOURCE_DIR}/docker/dependencies.versions")
set(_want "")
if(EXISTS "${_versions_file}")
  file(STRINGS "${_versions_file}" _want REGEX "^gersemi:")
  string(REGEX REPLACE "^gersemi:" "" _want "${_want}")
endif()

# Unversioned only when that file could not be read, where naming no version
# beats naming a wrong one.
if(_want)
  set(_install_spec "gersemi==${_want}")
else()
  set(_install_spec "gersemi")
endif()

# GERSEMI is whatever the configure-time find_program cached, which is empty
# or <var>-NOTFOUND if gersemi was not on PATH then.  Look again here, in the
# environment this target is actually run in, so that installing gersemi
# after configuring -- or putting it on PATH for one command -- is enough on
# its own.  Re-running cmake would find it too, but nothing tells the user
# their build tree is the thing holding the stale answer.
if(NOT GERSEMI OR GERSEMI MATCHES "NOTFOUND$")
  find_program(_gersemi_on_path NAMES gersemi)
  if(_gersemi_on_path)
    set(GERSEMI "${_gersemi_on_path}")
  endif()
endif()

if(NOT GERSEMI OR GERSEMI MATCHES "NOTFOUND$")
  dyninst_gersemi_section("installing gersemi")
  message(
    "The cmake-check, cmake-check-raw, cmake-reformat and\n"
    "cmake-reformat-diff targets format this project's CMake code with\n"
    "gersemi.  Install it with\n"
    "\n"
    "    python3 -m pip install ${_install_spec}\n"
    "\n"
    "and run this again, or name a copy that is not on PATH with\n"
    "\n"
    "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<path> <build-dir>\n"
  )
  message(FATAL_ERROR "gersemi was not found")
endif()

set(_have "")
execute_process(
  COMMAND "${GERSEMI}" --version
  OUTPUT_VARIABLE _version_text
  ERROR_QUIET
  RESULT_VARIABLE _version_rc
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(_version_rc EQUAL 0 AND _version_text MATCHES "gersemi ([0-9][0-9a-zA-Z.]*)")
  set(_have "${CMAKE_MATCH_1}")
endif()

# Enforced only when both versions are known, so a future --version format
# this cannot parse degrades to no check rather than a false failure.
if(_want AND _have AND NOT _have STREQUAL _want)
  dyninst_gersemi_section("wrong gersemi version")
  message(
    "gersemi ${_have} is installed, but this project is formatted with "
    "${_want}.\n"
    "Different releases format differently, so ${_have} would rewrite files\n"
    "that are already correct.  Install the pinned version with\n"
    "\n"
    "    python3 -m pip install ${_install_spec}\n"
    "\n"
    "The version is recorded in docker/dependencies.versions.\n"
  )
  message(FATAL_ERROR "gersemi ${_have} installed, ${_want} required")
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
  # gersemi exits non-zero for a file it could not even parse as well as for
  # one that is merely unformatted, and only the second is what cmake-reformat
  # is for.  In check mode _reformat_count is the count of files it actually
  # flagged, so a zero there with a non-zero exit means the reason is in the
  # warnings section and the generic message below is the true one.  check-raw
  # hands gersemi's own report straight through without reading it, so there
  # is nothing to tell the two apart and the advice stands as a guess.
  if(MODE STREQUAL "check-raw" OR _reformat_count GREATER 0)
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
