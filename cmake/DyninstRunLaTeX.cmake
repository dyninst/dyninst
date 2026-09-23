# Run pdflatex until the document reaches a fixed point.
#
# LaTeX resolves the table of contents, cross references and page numbers
# through the .aux/.toc/.out files, which are written by one pass and read by
# the next, so a single pass is not enough and a fixed number of passes is
# either wasteful or wrong.  Repeat until those files stop changing and the log
# stops asking, then stop.
#
# Driven as: cmake -DPDFLATEX=... -DSOURCE_DIR=... -DOUTPUT_DIR=... -DMAIN=...
#                  [-DMAX_PASSES=n] -P DyninstRunLaTeX.cmake
#
# Nothing is written outside OUTPUT_DIR: pdflatex reads the document in place
# from SOURCE_DIR and -output-directory sends every file it creates elsewhere.

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
    return()
  endif()

  set(_previous "${_current}")
  math(EXPR _pass "${_pass} + 1")
endwhile()

message(WARNING
        "${MAIN}: still unsettled after ${MAX_PASSES} passes; "
        "cross references or page numbers may be stale")
