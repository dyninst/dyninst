# Run pdflatex until the document reaches a fixed point.
#
# LaTeX resolves the table of contents, cross references and page numbers
# through the .aux/.toc/.out files, which are written by one pass and read by
# the next, so a single pass is not enough and a fixed number of passes is
# either wasteful or wrong.  Repeat until those files stop changing and the log
# stops asking, then stop.
#
# After the last pass the log is checked, so that a document which builds but
# has started warning does not pass silently.  Reported are
#
#   LaTeX Warning        \\@latex@warning, the kernel's own
#   LaTeX Font Warning   \\@font@warning, a shape that had to be substituted
#   Package X Warning    \\PackageWarning, from any package
#   Class X Warning      \\ClassWarning
#   Module X Warning     \\ModuleWarning
#   pdfTeX warning       from pdfTeX rather than LaTeX
#   Missing character    a glyph absent from the font, dropped from the output
#
# every overfull \hbox wider than MAX_OVERFULL_PT - the point at which a line
# stops protruding into the margin and starts running off the paper - and every
# overfull \vbox, whatever its size.
#
# Two project options change that, matching what they mean for the C++ build:
#
#   DISABLE_SUPPRESSIONS  report everything.  ALLOW_WARNINGS is ignored, and
#                         every overfull and underfull box is listed however
#                         small, not just the ones that leave the paper.
#
# A box that does leave the paper is tagged [RENDERS OFF PAGE], so it stands
# out among the many harmless ones that DISABLE_SUPPRESSIONS brings with it.
#
#   WARNINGS_AS_ERRORS    fail the target on whatever was reported.  Off, the
#                         diagnostics are printed and the build carries on, so
#                         a warning that only appears on some other TeX
#                         installation does not stop everything else building.
#
# A pdflatex failure is always fatal, whichever way those are set.
#
# ALLOW_WARNINGS is a regular expression for the warnings a manual is known to
# produce.  Overfull \hboxes narrower than MAX_OVERFULL_PT are not reported by
# default: they are invisible, and their number moves with any edit that
# reflows a paragraph.
#
# Driven as: cmake -DPDFLATEX=... -DSOURCE_DIR=... -DOUTPUT_DIR=... -DMAIN=...
#                  [-DMAX_PASSES=n] [-DALLOW_WARNINGS=regex]
#                  [-DMAX_OVERFULL_PT=n] [-DWARNINGS_AS_ERRORS=bool]
#                  [-DDISABLE_SUPPRESSIONS=bool] -P DyninstRunLaTeX.cmake
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
  if(DISABLE_SUPPRESSIONS)
    set(_allow "") # report the warnings a manual is otherwise allowed
    set(_box_limit 0) # and every box, not only those off the paper
  else()
    set(_allow "${ALLOW_WARNINGS}")
    set(_box_limit "${MAX_OVERFULL_PT}")
  endif()

  set(_problems "")

  # Match on the log text rather than indexing a list of its lines: a
  # semicolon in the log is a list separator to CMake and would split a line
  # in two.  Escaping them keeps foreach safe.
  file(READ "${_log}" _txt)
  string(REPLACE ";" "\\;" _txt "${_txt}")
  string(ASCII 10 _nl)

  # A warning's text wraps at the log's line width, so take the line that
  # follows as well; and start at the keyword, because pdfTeX prints its
  # warnings in the middle of the page-progress output.
  string(REGEX MATCHALL
         "(LaTeX Warning|LaTeX Font Warning|(Package|Class|Module) [A-Za-z0-9@._-]+ Warning|pdfTeX warning|Missing character)[^${_nl}]*${_nl}[^${_nl}]*"
         _warnings "${_txt}")
  foreach(_w ${_warnings})
    string(REGEX REPLACE "[ \t]*${_nl}[ \t]*" " " _w "${_w}")
    if(NOT (_allow AND _w MATCHES "${_allow}"))
      string(APPEND _problems "\n    ${_w}")
    endif()
  endforeach()

  if(_box_limit EQUAL 0)
    string(REGEX MATCHALL "(Overfull|Underfull) [^${_nl}]*" _boxes "${_txt}")
  else()
    string(REGEX MATCHALL "Overfull .[hv]box \\([0-9.]+pt[^${_nl}]*" _boxes "${_txt}")
  endif()
  foreach(_b ${_boxes})
    set(_off "")
    set(_report FALSE)
    if(_box_limit EQUAL 0)
      set(_report TRUE)
    endif()

    # A box overflowing by more than the margin does not merely reach into it,
    # it prints past the trim.  Say so: with suppressions off that is the only
    # reason an overfull \hbox is listed, and with them on it is what
    # distinguishes the few that matter from the many that do not.  The margin
    # is the same in both directions at margin=1in, so one threshold serves.
    if(_b MATCHES "^Overfull .[hv]box \\(([0-9.]+)pt"
       AND CMAKE_MATCH_1 GREATER MAX_OVERFULL_PT)
      set(_off " [RENDERS OFF PAGE]")
      set(_report TRUE)
    endif()

    # An overfull \vbox is reported however small.  Horizontally the trim is
    # the only thing a line can run into, so a box that stays within the
    # margin is invisible; vertically the page number sits \footskip (30pt)
    # below the text block, so vertical overflow collides with the footer well
    # before it reaches the paper's edge.  There is no width below which one
    # is harmless, and they are rare enough to report unconditionally.
    if(_b MATCHES "^Overfull .vbox")
      set(_report TRUE)
    endif()

    if(_report)
      string(APPEND _problems "\n    ${_b}${_off}")
    endif()
  endforeach()

  if(NOT _problems)
    return()
  endif()

  set(_msg "${MAIN}: the document built but the log is not clean:${_problems}\n"
           "  full log: ${_log}")
  if(WARNINGS_AS_ERRORS)
    message(FATAL_ERROR "${_msg}\n"
            "  Set -DDYNINST_WARNINGS_AS_ERRORS=OFF to report these without "
            "failing, or add one to ALLOW_WARNINGS in the manual's "
            "doc/CMakeLists.txt.")
  else()
    message(WARNING "${_msg}")
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
