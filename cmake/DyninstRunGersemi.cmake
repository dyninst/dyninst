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
# It exits 0 when gersemi succeeds, 2 when check mode finds unformatted files
# (on CMake 3.29 or later, which can choose its exit status), and 1 for any
# other failure.
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

# A section is its title between two rules, two blank lines after the previous
# one, so a file list of any length does not run into what follows.  Headings
# are plain message(), not STATUS: STATUS writes to stdout while everything
# else here is on stderr, and splitting the streams separates a section from
# its heading as soon as either is redirected.
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

# The oldest Python the pinned release installs on (requires_python on PyPI),
# named in every install hint below.  It is a floor for this formatting
# tooling only, not for building Dyninst.  Check it when bumping the pin,
# along with the same figure in .github/workflows/README.md.
set(_python_floor "3.8")

# How to install it, shared by every section that asks for it.  Debian 12,
# Ubuntu 23.04, Fedora 38 and later mark the system Python as externally
# managed (PEP 668), and pip then refuses to install into it, with --user
# or without; pipx, or a venv, is the way around that.
string(
  CONCAT _install_hint
  "    python3 -m pip install ${_install_spec}\n"
  "\n"
  "The pinned gersemi needs Python ${_python_floor} or later.  Where pip refuses\n"
  "with \"externally-managed-environment\", install gersemi with pipx\n"
  "instead, or into a venv:\n"
  "\n"
  "    pipx install ${_install_spec}\n"
)

# GERSEMI is whatever the configure-time find_program cached: empty or
# <var>-NOTFOUND if gersemi was not on PATH then, or a path that has since
# gone away with the venv it was in.  Look again here, in the environment
# this target is actually run in, so that installing gersemi after
# configuring -- or putting it on PATH for one command -- is enough on its
# own.  Re-running cmake would find it too, but nothing tells the user their
# build tree is the thing holding the stale answer.
#
# A path that does not exist may also be one the user typed, and this script
# cannot tell it from a cached one: both arrive as GERSEMI.  So it is never
# replaced silently -- the report names it, whether a gersemi on PATH stands
# in for it or nothing does.
set(_gersemi_missing "")
if(GERSEMI AND NOT EXISTS "${GERSEMI}")
  set(_gersemi_missing "${GERSEMI}")
  set(GERSEMI "")
endif()
if(NOT GERSEMI)
  find_program(_gersemi_on_path NAMES gersemi)
  if(_gersemi_on_path)
    set(GERSEMI "${_gersemi_on_path}")
  endif()
endif()

if(_gersemi_missing AND GERSEMI)
  dyninst_gersemi_section("using the gersemi on PATH")
  message(
    "DYNINST_GERSEMI_EXECUTABLE names\n"
    "\n"
    "    ${_gersemi_missing}\n"
    "\n"
    "which was not found, so this run uses the gersemi on PATH instead:\n"
    "\n"
    "    ${GERSEMI}\n"
    "\n"
    "If the path was mistyped, correct it with\n"
    "\n"
    "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<path> <build-dir>\n"
    "\n"
    "If an earlier configure found that path and the gersemi there has since\n"
    "been removed, re-running cmake on the build directory picks up the one\n"
    "on PATH.\n"
  )
elseif(_gersemi_missing)
  dyninst_gersemi_section("finding gersemi")
  message(
    "DYNINST_GERSEMI_EXECUTABLE names\n"
    "\n"
    "    ${_gersemi_missing}\n"
    "\n"
    "which was not found, and there is no gersemi on PATH either.  If the\n"
    "path was mistyped, correct it with\n"
    "\n"
    "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<path> <build-dir>\n"
    "\n"
    "or install the pinned version with\n"
    "\n"
    "${_install_hint}"
  )
  message(FATAL_ERROR "DYNINST_GERSEMI_EXECUTABLE names a gersemi that was not found")
elseif(NOT GERSEMI)
  dyninst_gersemi_section("installing gersemi")
  message(
    "The cmake-check, cmake-check-raw, cmake-reformat and\n"
    "cmake-reformat-diff targets format this project's CMake code with\n"
    "gersemi.  Install the pinned version with\n"
    "\n"
    "${_install_hint}"
    "\n"
    "Then run this again, or name a copy that is not on PATH with\n"
    "\n"
    "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<path> <build-dir>\n"
  )
  message(FATAL_ERROR "gersemi was not found")
endif()

set(_have "")
execute_process(
  COMMAND "${GERSEMI}" --version
  OUTPUT_VARIABLE _version_text
  ERROR_VARIABLE _version_error
  RESULT_VARIABLE _version_rc
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

# A gersemi that cannot answer --version cannot format anything either, and
# letting it go on only files its traceback under "other warnings" as if it
# were a formatting problem.
#
# The commonest failure is a pip or pipx launcher whose Python cannot import
# the package, or is not there at all -- execute_process then reports "no
# such file or directory" for a file that exists, with nothing on stderr.
# CMake before 3.28 capitalises it, so both spellings are matched.
# Mostly the package was installed for another Python: on a home
# directory shared between machines, ~/.local/bin holds one launcher for all
# of them, and each machine's pip or pipx overwrites the last one's.  The
# launcher still exists, so the search above keeps it.  Removing it does not
# help on its own -- pip that says "Requirement already satisfied" does not
# write it again -- so the advice is a venv per machine.
if(NOT _version_rc EQUAL 0)
  if(_version_rc MATCHES "^[0-9]+$")
    set(_version_why "exited with ${_version_rc}")
  else()
    set(_version_why "failed: ${_version_rc}")
  endif()
  dyninst_gersemi_section("running gersemi")
  if(_version_error STREQUAL "")
    message("${GERSEMI} --version ${_version_why}\n")
  else()
    string(REPLACE "\n" "\n  " _version_error "  ${_version_error}")
    message("${GERSEMI} --version failed:\n" "\n" "${_version_error}\n")
  endif()
  if(
    _version_error MATCHES "ModuleNotFoundError|No module named"
    OR _version_rc MATCHES "^[Nn]o such file or directory$"
  )
    message(
      "The gersemi there is a launcher whose Python, named on its first\n"
      "line, is missing or cannot import the gersemi package.  Usually the\n"
      "package was installed for a different Python -- often on another\n"
      "machine that shares this home directory, where ~/.local/bin holds a\n"
      "single launcher for every machine -- and sometimes it has been\n"
      "removed.\n"
      "\n"
      "Where the home directory is shared, give each machine its own\n"
      "gersemi, in a venv outside it, and point the build tree at that:\n"
      "\n"
      "    python3 -m venv <dir>\n"
      "    <dir>/bin/pip install ${_install_spec}\n"
      "    cmake -DDYNINST_GERSEMI_EXECUTABLE=<dir>/bin/gersemi <build-dir>\n"
      "\n"
      "The pinned gersemi needs Python ${_python_floor} or later.  Where the home\n"
      "directory is not shared, reinstalling for the launcher's own Python\n"
      "also works, with --force-reinstall: pip that reports \"Requirement\n"
      "already satisfied\" leaves the launcher as it is.\n"
    )
  else()
    message(
      "The gersemi there cannot run.  Install the pinned version with\n"
      "\n"
      "${_install_hint}"
    )
  endif()
  message(FATAL_ERROR "gersemi --version ${_version_why}")
endif()

if(_version_text MATCHES "gersemi ([0-9][0-9a-zA-Z.]*)")
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
    "${_install_hint}"
    "\n"
    "The version is recorded in docker/dependencies.versions.  On a python3\n"
    "older than ${_python_floor}, pip reports no matching distribution for gersemi\n"
    "${_want}, and an unversioned install quietly picks an older gersemi\n"
    "instead.\n"
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

# Without quotePath=false git C-quotes any path with a non-ASCII byte in it,
# and the EXISTS filter below would then drop that file without a word.
execute_process(
  COMMAND "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" -c core.quotePath=false ls-files
  OUTPUT_VARIABLE _tracked
  ERROR_VARIABLE _git_error
  RESULT_VARIABLE _git_result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT _git_result EQUAL 0)
  message(FATAL_ERROR "'git ls-files' failed in ${SOURCE_DIR}: ${_git_error}")
endif()

# The file list is a CMake list, so a ';' in a path splits it in two and an
# unbalanced '[' joins it to every path up to the next ']'.  The EXISTS filter
# below would drop the pieces without a word, and no such path could reach
# gersemi as one argument through a list anyway.  Refuse it rather than skip
# it.  That also refuses a balanced '[...]', which would work; one rule that
# is easy to state is worth that.
if(_tracked MATCHES "(^|\n)([^\n]*[;[][^\n]*(CMakeLists\\.txt|\\.cmake))(\n|$)")
  message(FATAL_ERROR "a tracked CMake file has ';' or '[' in its path: ${CMAKE_MATCH_2}")
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
  # Unknown-command warnings belong to cmake-check, which has a section for
  # them; interleaved with the diff they are noise that changes none of it.
  set(_flags --diff --no-warn-about-unknown-commands)
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
  # carries the information; gersemi reports absolute paths even when it was
  # given relative ones.
  #
  # Splitting into lines goes through a CMake list, which gives meaning to
  # characters gersemi's output may well contain: a ';' would split a line,
  # a '\' would escape the separator after it, and an unbalanced '[' would
  # join every line up to the next ']'.  They are swapped for control
  # characters that do not occur in that output, and swapped back only in the
  # finished text, once no list is involved any more.
  string(ASCII 1 _esc_semicolon)
  string(ASCII 2 _esc_backslash)
  string(ASCII 3 _esc_open)
  string(ASCII 4 _esc_close)
  set(_text "${_stdout}${_stderr}")
  string(REPLACE "${SOURCE_DIR}/" "" _text "${_text}")
  string(REPLACE ";" "${_esc_semicolon}" _text "${_text}")
  string(REPLACE "\\" "${_esc_backslash}" _text "${_text}")
  string(REPLACE "[" "${_esc_open}" _text "${_text}")
  string(REPLACE "]" "${_esc_close}" _text "${_text}")
  string(REPLACE "\n" ";" _lines "${_text}")

  macro(dyninst_gersemi_unescape _var)
    string(REPLACE "${_esc_semicolon}" ";" ${_var} "${${_var}}")
    string(REPLACE "${_esc_backslash}" "\\" ${_var} "${${_var}}")
    string(REPLACE "${_esc_open}" "[" ${_var} "${${_var}}")
    string(REPLACE "${_esc_close}" "]" ${_var} "${${_var}}")
  endmacro()

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
  dyninst_gersemi_unescape(_warnings)
  list(LENGTH _reformat _reformat_count)

  # gersemi hands its files to a pool of workers, so the order it reports
  # them in changes from run to run.  Sorting makes two runs comparable.
  if(_reformat_count GREATER 0)
    list(SORT _reformat)
    list(JOIN _reformat "\n  " _joined)
    dyninst_gersemi_unescape(_joined)
    dyninst_gersemi_section("gersemi --check on ${_count} files: reformatting required")
    message("  ${_joined}")
  endif()

  if(NOT _warnings STREQUAL "")
    dyninst_gersemi_section("gersemi --check on ${_count} files: other warnings")
    message("${_warnings}")
  endif()

  # Only on success: gersemi can also fail without a word, when it is killed
  # or cannot be run at all, and then the exit status below is the report.
  if(_result EQUAL 0 AND _reformat_count EQUAL 0 AND _warnings STREQUAL "")
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
    # Exit 2 rather than FATAL_ERROR's 1 when check mode has seen the
    # unformatted files for itself, so the CMake Formatting workflow can show
    # the diff for exactly this failure and not for one a diff cannot explain.
    # cmake_language(EXIT) needs 3.29; on older CMake every failure is a 1.
    if(MODE STREQUAL "check" AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.29)
      message("the CMake code is not formatted")
      cmake_language(EXIT 2)
    endif()
    message(FATAL_ERROR "the CMake code is not formatted")
  else()
    message(FATAL_ERROR "gersemi ${_flags_text} exited with ${_result}")
  endif()
endif()
