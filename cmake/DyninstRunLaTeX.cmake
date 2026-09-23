# Run pdflatex until the document reaches a fixed point.
#
# LaTeX resolves the table of contents, cross references and page numbers
# through the .aux/.toc/.out files, which are written by one pass and read by
# the next, so a single pass is not enough and a fixed number of passes is
# either wasteful or wrong.  Repeat until those files stop changing and the log
# stops asking, then stop.
#
# After the last pass the log is checked, so that a document which builds but
# has started warning does not pass silently:
#
#   every LaTeX, LaTeX Font or pdfTeX warning is an error, unless it matches
#   ALLOW_WARNINGS, a regular expression for the ones a manual is known to
#   produce; and
#
#   an overfull box wider than MAX_OVERFULL_PT is an error, that being the
#   point at which a line stops protruding into the margin and starts running
#   off the paper.  Narrower boxes are not counted: they are invisible, and
#   their number moves with any edit that reflows a paragraph.
#
# Driven as: cmake -DPDFLATEX=... -DSOURCE_DIR=... -DOUTPUT_DIR=... -DMAIN=...
#                  [-DMAX_PASSES=n] [-DALLOW_WARNINGS=regex]
#                  [-DMAX_OVERFULL_PT=n] -P DyninstRunLaTeX.cmake
#
# Nothing is written outside OUTPUT_DIR: pdflatex reads the document in place
# from SOURCE_DIR and -output-directory sends every file it creates elsewhere.

# Blank lines in the log are empty list elements; CMP0007 keeps them rather
# than silently dropping them, which is what the line indexing below needs.
cmake_minimum_required(VERSION 3.14.0)

foreach(_v PDFLATEX SOURCE_DIR OUTPUT_DIR MAIN)
  if(NOT DEFINED ${_v})
    message(FATAL_ERROR "DyninstRunLaTeX: ${_v} must be set")
  endif()
endforeach()

if(NOT DEFINED MAX_PASSES)
  set(MAX_PASSES 5)
endif()

get_filename_component(_stem "${MAIN}" NAME_WE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

# The files that carry state from one pass to the next.
set(_state_files
    "${OUTPUT_DIR}/${_stem}.aux" "${OUTPUT_DIR}/${_stem}.toc"
    "${OUTPUT_DIR}/${_stem}.out")

function(_dyninst_latex_state out)
  set(_s "")
  foreach(_f ${_state_files})
    if(EXISTS "${_f}")
      file(MD5 "${_f}" _h)
      string(APPEND _s "${_f}:${_h};")
    endif()
  endforeach()
  set(${out} "${_s}" PARENT_SCOPE)
endfunction()

function(_dyninst_latex_check _log)
  # An empty -D still defines the variable, so test the value, not DEFINED.
  if(MAX_OVERFULL_PT STREQUAL "")
    set(MAX_OVERFULL_PT 72.27) # the right margin, in points, at 1in margins
  endif()

  set(_problems "")

  # Read the log as one string and split it by hand: file(STRINGS) would
  # treat a semicolon in the log as a list separator and mangle the lines.
  file(READ "${_log}" _txt)
  string(REPLACE ";" "\\;" _txt "${_txt}")
  string(REPLACE "\n" ";" _lines "${_txt}")
  list(LENGTH _lines _n)

  # A warning's text wraps at the log's line width, so test it together with
  # the line that follows; matching the first line alone would miss whatever
  # the wrap pushed onto the second.
  set(_i 0)
  while(_i LESS _n)
    list(GET _lines ${_i} _l)
    if(_l MATCHES "^LaTeX Warning|^LaTeX Font Warning|pdfTeX warning")
      math(EXPR _j "${_i} + 1")
      if(_j LESS _n)
        list(GET _lines ${_j} _next)
        set(_full "${_l} ${_next}")
      else()
        set(_full "${_l}")
      endif()
      if(NOT (ALLOW_WARNINGS AND _full MATCHES "${ALLOW_WARNINGS}"))
        string(APPEND _problems "\n    ${_full}")
      endif()
    endif()
    math(EXPR _i "${_i} + 1")
  endwhile()

  if(MAX_OVERFULL_PT GREATER 0)
    file(STRINGS "${_log}" _boxes REGEX "^Overfull .hbox \\(")
    foreach(_b ${_boxes})
      if(_b MATCHES "^Overfull .hbox \\(([0-9.]+)pt")
        if(CMAKE_MATCH_1 GREATER MAX_OVERFULL_PT)
          string(APPEND _problems "\n    ${_b}")
        endif()
      endif()
    endforeach()
  endif()

  if(_problems)
    message(FATAL_ERROR
            "${MAIN}: the document built but the log is not clean:${_problems}\n"
            "  full log: ${_log}\n"
            "  If one of these is expected, add it to ALLOW_WARNINGS for this "
            "manual in its doc/CMakeLists.txt.")
  endif()
endfunction()

# Seed the comparison with whatever a previous build left behind, so a
# document that is already settled needs one confirming pass, not two.
_dyninst_latex_state(_previous)
set(_pass 1)
while(_pass LESS_EQUAL MAX_PASSES)
  execute_process(
    COMMAND "${PDFLATEX}" -interaction=nonstopmode -file-line-error
            -output-directory=${OUTPUT_DIR} ${MAIN}
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _rc
    OUTPUT_QUIET ERROR_QUIET)

  if(NOT _rc EQUAL 0)
    # nonstopmode still exits non-zero on a LaTeX error, so a broken document
    # fails the build instead of quietly producing a damaged PDF.
    set(_log "${OUTPUT_DIR}/${_stem}.log")
    set(_detail "")
    if(EXISTS "${_log}")
      file(STRINGS "${_log}" _lines REGEX "^[^ ]*\\.(tex|sty|cls|aux|toc):[0-9]+:")
      list(LENGTH _lines _n)
      if(_n GREATER 0)
        list(SUBLIST _lines 0 1 _first)
        set(_detail "\n  ${_n} LaTeX error(s), first: ${_first}")
      endif()
    endif()
    message(FATAL_ERROR
            "pdflatex failed on ${MAIN} (pass ${_pass}, exit ${_rc})${_detail}\n"
            "  full log: ${_log}")
  endif()

  _dyninst_latex_state(_current)

  set(_rerun FALSE)
  file(STRINGS "${OUTPUT_DIR}/${_stem}.log" _asks
       REGEX "Rerun to get|Rerun LaTeX|Label\\(s\\) may have changed")
  if(_asks)
    set(_rerun TRUE)
  endif()

  if(_current STREQUAL _previous AND NOT _rerun)
    _dyninst_latex_check("${OUTPUT_DIR}/${_stem}.log")
    return()
  endif()

  set(_previous "${_current}")
  math(EXPR _pass "${_pass} + 1")
endwhile()

message(WARNING
        "${MAIN}: still unsettled after ${MAX_PASSES} passes; "
        "cross references or page numbers may be stale")
