# Run pdflatex until the document reaches a fixed point.
#
# The .aux/.toc/.out files are written by one pass and read by the next, so a
# fixed number of passes is either wasteful or wrong: repeat until they stop
# changing and the log stops asking.
#
# The log is then checked.  Reported are
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
# Two project options change that, as they do for the C++ build:
#
#   DISABLE_SUPPRESSIONS  report everything: ALLOW_WARNINGS ignored, and every
#                         box however small.  A box that leaves the paper is
#                         tagged [RENDERS OFF PAGE] so it still stands out.
#   WARNINGS_AS_ERRORS    fail the target on whatever was reported.  Off, the
#                         diagnostics are printed and the build carries on.
#
# A pdflatex failure is always fatal.  ALLOW_WARNINGS is a regular expression
# for the warnings a manual is known to produce.  Overfull \hboxes narrower
# than MAX_OVERFULL_PT are not reported by default: they are invisible, and
# their number moves with any edit that reflows a paragraph.
#
# Driven as: cmake -DPDFLATEX=... -DSOURCE_DIR=... -DOUTPUT_DIR=... -DMAIN=...
#                  [-DMAX_PASSES=n] [-DALLOW_WARNINGS=regex]
#                  [-DMAX_OVERFULL_PT=n] [-DWARNINGS_AS_ERRORS=bool]
#                  [-DDISABLE_SUPPRESSIONS=bool] [-DGHOSTSCRIPT=path]
#                  -P DyninstRunLaTeX.cmake
#
# GHOSTSCRIPT is optional.  Given, the finished PDF is measured as well as the
# log, and a page whose ink reaches the paper's edge is reported.
#
# Nothing is written outside OUTPUT_DIR: pdflatex reads the document in place
# from SOURCE_DIR and -output-directory sends every file it creates elsewhere.

# Match the project's minimum, so this script sees the same policy defaults
# as the build that invokes it.
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
set(_state_files "${OUTPUT_DIR}/${_stem}.aux" "${OUTPUT_DIR}/${_stem}.toc"
                 "${OUTPUT_DIR}/${_stem}.out")

function(_dyninst_latex_state out)
  set(_s "")
  foreach(_f ${_state_files})
    if(EXISTS "${_f}")
      file(MD5 "${_f}" _h)
      string(APPEND _s "${_f}:${_h};")
    endif()
  endforeach()
  set(${out}
      "${_s}"
      PARENT_SCOPE)
endfunction()

function(_dyninst_latex_check _log)
  # An empty -D still defines the variable, so the value has to be tested; and
  # an undefined name compares as the literal string "MAX_OVERFULL_PT", never
  # as "", so DEFINED has to be tested too.  Both spellings reach the default.
  if(NOT DEFINED MAX_OVERFULL_PT OR MAX_OVERFULL_PT STREQUAL "")
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

  # One warning per match: the run sets max_print_line, so the keyword line
  # holds all of it.  Do not take the following line as well -- MATCHALL does
  # not overlap, so two adjacent warnings would merge into one match and
  # ALLOW_WARNINGS matching the first would hide the second.
  #
  # LaTeX continues some warnings itself, tagging each line with the package
  # that raised it: "(Font)", "(hyperref)".  Those are taken too, told from a
  # file-open line like "(./3-API.tex" by the closing paren after a bare name.
  # Start at the keyword; pdfTeX prints warnings amid its page progress.
  string(
    REGEX
      MATCHALL
      "(LaTeX Warning|LaTeX Font Warning|(Package|Class|Module) [A-Za-z0-9@._-]+ Warning|pdfTeX warning|Missing character)[^${_nl}]*(${_nl}\\([A-Za-z][A-Za-z0-9@._-]*\\)[^${_nl}]*)*"
      _warnings
      "${_txt}")
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

    # Overflowing by more than the margin means printing past the trim, not
    # merely into the margin.  One threshold serves both directions at
    # margin=1in.
    if(_b MATCHES "^Overfull .[hv]box \\(([0-9.]+)pt" AND CMAKE_MATCH_1 GREATER
                                                          MAX_OVERFULL_PT)
      set(_off " [RENDERS OFF PAGE]")
      set(_report TRUE)
    endif()

    # An overfull \vbox is reported however small: the page number sits
    # \footskip (30pt) below the text block, so vertical overflow hits the
    # footer long before the trim.  There is no harmless width.
    if(_b MATCHES "^Overfull .vbox")
      set(_report TRUE)
    endif()

    if(_report)
      string(APPEND _problems "\n    ${_b}${_off}")
    endif()
  endforeach()

  set(_check_problems
      "${_problems}"
      PARENT_SCOPE)
endfunction()

# Pull the numbers out of a line Ghostscript printed, as whole big points.
# Truncating sidesteps arithmetic CMake 3.14's math() cannot do on decimals,
# and a whole bp is far finer than any clearance being tested for.
function(_dyninst_points _text _out)
  string(REGEX MATCHALL "-?[0-9]+(\\.[0-9]+)?" _tokens "${_text}")
  set(_r "")
  foreach(_t ${_tokens})
    string(REGEX REPLACE "\\..*$" "" _t "${_t}")
    list(APPEND _r "${_t}")
  endforeach()
  set(${_out}
      "${_r}"
      PARENT_SCOPE)
endfunction()

# TeX points (1in = 72.27pt) to PostScript points (1in = 72bp), rounded.
# Worked in thousandths because integer arithmetic is all CMake 3.14's math()
# offers.  "1${_f} - 1000" rather than "${_f}" so a fraction like 069 is not
# read as octal.
function(_dyninst_pt_to_bp _pt _out)
  set(${_out}
      ""
      PARENT_SCOPE)
  if(NOT _pt MATCHES "^([0-9]+)\\.?([0-9]*)$")
    return()
  endif()
  set(_i "${CMAKE_MATCH_1}")
  set(_f "${CMAKE_MATCH_2}000")
  string(SUBSTRING "${_f}" 0 3 _f)
  math(EXPR _milli "${_i} * 1000 + 1${_f} - 1000")
  math(EXPR _bp "(${_milli} * 7200 / 7227 + 500) / 1000")
  set(${_out}
      "${_bp}"
      PARENT_SCOPE)
endfunction()

# Measure where the ink lands.  The log cannot settle it: pdflatex reports a
# box's overflow relative to its enclosing box, never its position on the page,
# so overflows that compose - an over-wide table whose cell also overflows -
# each stay under the threshold while their sum prints past the trim.
#
# Ghostscript clips at the page box, so ink that runs off the paper gives a
# bounding box reaching the edge.  It cannot say how far past, only that it got
# there, which is all this decides.  Weakest vertically, where the last line
# above the edge can stop short of it and leave the box looking healthy -- why
# an overfull \vbox is reported unconditionally by _dyninst_latex_check.
function(_dyninst_latex_validate _pdf _log)
  set(_p "")

  # How close the ink may come to the edge of the paper before the page is
  # called a defect -- its clearance to the trim.  In big points: 1/72in, what
  # Ghostscript reports, as against TeX's pt of 1/72.27in.  3bp is about 1mm.
  if(NOT DEFINED MIN_TRIM_CLEARANCE_BP OR MIN_TRIM_CLEARANCE_BP STREQUAL "")
    set(MIN_TRIM_CLEARANCE_BP 3)
  endif()

  # The page box comes from the log, not from the PDF: Ghostscript has no way
  # of reporting it that works across its releases, and geometry records it in
  # every log.  pdfTeX writes the MediaBox from these same lengths, so the box
  # measured against is the box the pages were made with.
  file(READ "${_log}" _ltxt)
  set(_px1 "")
  set(_py1 "")
  if(_ltxt MATCHES "\\* .paperwidth=([0-9.]+)pt")
    _dyninst_pt_to_bp("${CMAKE_MATCH_1}" _px1)
  endif()
  if(_ltxt MATCHES "\\* .paperheight=([0-9.]+)pt")
    _dyninst_pt_to_bp("${CMAKE_MATCH_1}" _py1)
  endif()
  if(NOT _px1 OR NOT _py1)
    set(_validate_problems
        "\n    no page size in the log; is the geometry package loaded?"
        PARENT_SCOPE)
    return()
  endif()
  execute_process(
    COMMAND "${GHOSTSCRIPT}" -q -dBATCH -dNOPAUSE -sDEVICE=bbox "${_pdf}"
    OUTPUT_QUIET
    ERROR_VARIABLE _bb
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    set(_p "\n    Ghostscript could not measure the PDF")
    set(_validate_problems
        "${_p}"
        PARENT_SCOPE)
    return()
  endif()

  string(REGEX MATCHALL "%%HiResBoundingBox:[0-9. ]+" _pages "${_bb}")
  set(_pageno 0)
  foreach(_box ${_pages})
    math(EXPR _pageno "${_pageno} + 1")
    string(REGEX REPLACE "^[^:]*:" "" _box "${_box}")
    _dyninst_points("${_box}" _edges)
    list(LENGTH _edges _n2)
    if(NOT _n2 EQUAL 4)
      continue()
    endif()
    list(GET _edges 0 _llx)
    list(GET _edges 1 _lly)
    list(GET _edges 2 _urx)
    list(GET _edges 3 _ury)

    # Ghostscript reports a page with no ink as 0 0 0 0.  A page is blank on
    # purpose often enough -- \cleardoublepage leaves one -- and reading that
    # as ink at the origin says it reaches the left and bottom edges.
    if(_llx EQUAL 0
       AND _lly EQUAL 0
       AND _urx EQUAL 0
       AND _ury EQUAL 0)
      continue()
    endif()

    # pdfTeX puts the page's origin at the corner of the paper, so the ink's
    # lower-left coordinates are already its clearances from those two sides.
    set(_left "${_llx}")
    set(_bottom "${_lly}")
    math(EXPR _right "${_px1} - (${_urx})")
    math(EXPR _top "${_py1} - (${_ury})")

    set(_sides "")
    if(_left LESS MIN_TRIM_CLEARANCE_BP)
      list(APPEND _sides "left")
    endif()
    if(_right LESS MIN_TRIM_CLEARANCE_BP)
      list(APPEND _sides "right")
    endif()
    if(_bottom LESS MIN_TRIM_CLEARANCE_BP)
      list(APPEND _sides "bottom")
    endif()
    if(_top LESS MIN_TRIM_CLEARANCE_BP)
      list(APPEND _sides "top")
    endif()
    if(_sides)
      string(REPLACE ";" ", " _sides "${_sides}")
      string(APPEND _p "\n    page ${_pageno}: ink reaches the ${_sides} edge"
             " (clearance L${_left} R${_right} T${_top} B${_bottom} bp)")
    endif()
  endforeach()

  set(_validate_problems
      "${_p}"
      PARENT_SCOPE)
endfunction()

# Called for a document that settled and for one that ran out of passes: the
# unsettled one is the more likely to be wrong, so it is checked like any other.
function(_dyninst_latex_report _unsettled)
  set(_check_problems "")
  set(_validate_problems "")
  _dyninst_latex_check("${OUTPUT_DIR}/${_stem}.log")
  if(GHOSTSCRIPT)
    _dyninst_latex_validate("${OUTPUT_DIR}/${_stem}.pdf" "${OUTPUT_DIR}/${_stem}.log")
  endif()

  set(_report "")
  if(_unsettled)
    string(APPEND _report "\n  it never settled: ${MAX_PASSES} passes left the"
           " cross references or page numbers still moving, so the table of"
           " contents and every \\ref may name the wrong page")
  endif()
  if(_check_problems)
    string(APPEND _report "\n  the log is not clean:${_check_problems}"
           "\n  full log: ${OUTPUT_DIR}/${_stem}.log")
  endif()
  if(_validate_problems)
    string(APPEND _report "\n  the page does not hold its content:${_validate_problems}")
  endif()
  if(NOT _report)
    return()
  endif()

  if(WARNINGS_AS_ERRORS)
    # ALLOW_WARNINGS only ever silences a log warning, so offer it only when
    # one is what failed.  A page that does not hold its content, or a
    # document that never settled, has to be fixed in the document --
    # or, for the latter, given more passes.
    set(_hint "")
    if(_check_problems)
      string(APPEND _hint " or add one to ALLOW_WARNINGS in the manual's"
             " manual-latex/CMakeLists.txt")
    endif()
    if(_unsettled)
      string(APPEND _hint " or raise MAX_PASSES in cmake/DyninstRunLaTeX.cmake"
             " if the document is simply large")
    endif()
    message(
      FATAL_ERROR
        "${MAIN}: the document built but${_report}\n"
        "  Set -DDYNINST_WARNINGS_AS_ERRORS=OFF to report these "
        "without failing${_hint}.")
  else()
    message(WARNING "${MAIN}: the document built but${_report}")
  endif()
endfunction()

# Seed the comparison with whatever a previous build left behind, so a
# document that is already settled needs one confirming pass, not two.
_dyninst_latex_state(_previous)
set(_pass 1)
while(_pass LESS_EQUAL MAX_PASSES)
  execute_process(
    # max_print_line stops pdfTeX wrapping its output at 79 columns, mid-word,
    # which would split a warning across two lines.  The log scan depends on
    # one warning per line.
    COMMAND
      ${CMAKE_COMMAND} -E env max_print_line=10000 "${PDFLATEX}" -interaction=nonstopmode
      -file-line-error -output-directory=${OUTPUT_DIR} ${MAIN}
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
    message(
      FATAL_ERROR "pdflatex failed on ${MAIN} (pass ${_pass}, exit ${_rc})${_detail}\n"
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
    _dyninst_latex_report(FALSE)
    return()
  endif()

  set(_previous "${_current}")
  math(EXPR _pass "${_pass} + 1")
endwhile()

_dyninst_latex_report(TRUE)
