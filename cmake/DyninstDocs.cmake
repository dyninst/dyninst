# Build the LaTeX manuals.
#
# Nothing is written into the source tree: pdflatex reads the document in
# place and -output-directory sends every file it produces into the build
# tree.  See cmake/DyninstRunLaTeX.cmake for the fixed-point driver.
#
# Targets
#   docs                 every manual
#   <module>.pdf         one manual
#   docs-install         install the manuals under CMAKE_INSTALL_DOCDIR
#
# DYNINST_BUILD_DOCS
#   ON   pdflatex is required; the manuals build as part of 'all' and are
#        installed by 'install'
#   OFF  the manuals build only when asked for by name; if pdflatex is
#        missing the targets still exist and fail with an explanation
#
# The log is checked after the build.  DYNINST_WARNINGS_AS_ERRORS decides
# whether what it reports fails the target or is only printed, and
# DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS reports everything rather than only
# what is not already known, exactly as they do for the C++ build.
#
# When Ghostscript is available the finished PDF is measured as well, because
# the log alone cannot prove a page is clean: pdflatex reports a box's overflow
# relative to whatever box encloses it, never its position on the page, so
# overflows that compose - an over-wide table whose cell also overflows - can
# each stay under the threshold while their sum prints past the trim.
# Measuring the ink settles it.  DYNINST_DOCS_FORCE_VALIDATE makes Ghostscript
# a hard requirement rather than letting the measurement be skipped silently.

include_guard(GLOBAL)
include(GNUInstallDirs)

find_package(LATEX COMPONENTS PDFLATEX)

if(DYNINST_BUILD_DOCS AND NOT LATEX_PDFLATEX_FOUND)
  message(FATAL_ERROR
          "DYNINST_BUILD_DOCS is ON but pdflatex was not found.\n"
          "  Install a LaTeX distribution, or configure with "
          "-DDYNINST_BUILD_DOCS=OFF to build the manuals on demand only.")
endif()

find_package(Ghostscript)

if(DYNINST_DOCS_FORCE_VALIDATE AND NOT Ghostscript_FOUND)
  message(FATAL_ERROR
          "DYNINST_DOCS_FORCE_VALIDATE is ON but Ghostscript was not found.\n"
          "  Install Ghostscript, or configure with "
          "-DDYNINST_DOCS_FORCE_VALIDATE=OFF to skip the measurement when it "
          "is unavailable.")
endif()

if(DYNINST_BUILD_DOCS)
  add_custom_target(docs ALL COMMENT "Building the Dyninst manuals")
else()
  add_custom_target(docs COMMENT "Building the Dyninst manuals")
endif()

set(DYNINST_DOCS_INSTALL_DIR
    "${CMAKE_INSTALL_DOCDIR}"
    CACHE STRING "Where 'install' and 'docs-install' put the manuals")

# Installing on demand.  install() is bound to the 'install' target, so route
# through the generated cmake_install.cmake with the component filter; that
# keeps DESTDIR and the install prefix working, which a plain copy would not.
add_custom_target(
  docs-install
  COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_COMPONENT=docs -P
          ${CMAKE_BINARY_DIR}/cmake_install.cmake
  COMMENT "Installing the Dyninst manuals into ${DYNINST_DOCS_INSTALL_DIR}")
add_dependencies(docs-install docs)

# ---------------------------------------------------------------------------
# dyninst_add_latex_document(
#     TARGET       <name>        target to create
#     SOURCE_DIR   <dir>         directory to run pdflatex in
#     MAIN         <file.tex>    document, relative to SOURCE_DIR
#     [OUTPUT_DIR  <dir>]        defaults to CMAKE_CURRENT_BINARY_DIR
#     [MAX_PASSES  <n>]          defaults to 5
#     [ALLOW_WARNINGS <regex>]   log warnings matching this are not errors
#     [MAX_OVERFULL_PT <n>]      overfull boxes wider than this are errors
#     [DEPENDS     <files>...]   rebuild when any of these change
#     [INSTALL_DESTINATION <d>]  install the PDF there, under component 'docs'
# )
#
# Everything is explicit; dyninst_add_manual() below fills most of it in from
# the layout these manuals share.
# ---------------------------------------------------------------------------
function(dyninst_add_latex_document)
  set(_opts "")
  set(_one TARGET SOURCE_DIR MAIN OUTPUT_DIR MAX_PASSES INSTALL_DESTINATION
          ALLOW_WARNINGS MAX_OVERFULL_PT)
  set(_many DEPENDS)
  cmake_parse_arguments(LTX "${_opts}" "${_one}" "${_many}" ${ARGN})

  foreach(_r TARGET SOURCE_DIR MAIN)
    if(NOT LTX_${_r})
      message(FATAL_ERROR "dyninst_add_latex_document: ${_r} is required")
    endif()
  endforeach()
  if(LTX_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
            "dyninst_add_latex_document: unrecognised: ${LTX_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT LTX_OUTPUT_DIR)
    set(LTX_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  endif()
  if(NOT LTX_MAX_PASSES)
    set(LTX_MAX_PASSES 5)
  endif()

  get_filename_component(_stem "${LTX_MAIN}" NAME_WE)
  set(_pdf "${LTX_OUTPUT_DIR}/${_stem}.pdf")

  if(NOT LATEX_PDFLATEX_FOUND)
    # Keep the target so the name always works; explain rather than fail
    # obscurely, and only when someone actually asks for it.
    add_custom_target(
      ${LTX_TARGET}
      COMMAND
        ${CMAKE_COMMAND} -E echo
        "error: cannot build ${_stem}.pdf - pdflatex was not found at configure time"
      COMMAND ${CMAKE_COMMAND} -E false
      COMMENT "Building ${_stem}.pdf")
    add_dependencies(docs ${LTX_TARGET})
    return()
  endif()

  add_custom_command(
    OUTPUT "${_pdf}"
    BYPRODUCTS "${LTX_OUTPUT_DIR}/${_stem}.aux" "${LTX_OUTPUT_DIR}/${_stem}.log"
               "${LTX_OUTPUT_DIR}/${_stem}.toc" "${LTX_OUTPUT_DIR}/${_stem}.out"
    COMMAND
      ${CMAKE_COMMAND} -DPDFLATEX=${PDFLATEX_COMPILER}
      -DSOURCE_DIR=${LTX_SOURCE_DIR} -DOUTPUT_DIR=${LTX_OUTPUT_DIR}
      -DMAIN=${LTX_MAIN} -DMAX_PASSES=${LTX_MAX_PASSES}
      -DALLOW_WARNINGS=${LTX_ALLOW_WARNINGS}
      -DMAX_OVERFULL_PT=${LTX_MAX_OVERFULL_PT}
      -DWARNINGS_AS_ERRORS=${DYNINST_WARNINGS_AS_ERRORS}
      -DDISABLE_SUPPRESSIONS=${DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS}
      -DGHOSTSCRIPT=${Ghostscript_EXECUTABLE} -P
      ${PROJECT_SOURCE_DIR}/cmake/DyninstRunLaTeX.cmake
    DEPENDS ${LTX_DEPENDS} ${PROJECT_SOURCE_DIR}/cmake/DyninstRunLaTeX.cmake
    COMMENT "Building ${_stem}.pdf"
    VERBATIM)

  add_custom_target(${LTX_TARGET} DEPENDS "${_pdf}")
  add_dependencies(docs ${LTX_TARGET})

  if(LTX_INSTALL_DESTINATION)
    install(
      FILES "${_pdf}"
      DESTINATION "${LTX_INSTALL_DESTINATION}"
      COMPONENT docs
      OPTIONAL)
  endif()
endfunction()

# ---------------------------------------------------------------------------
# dyninst_add_manual(<module> [ALLOW_WARNINGS <regex>] [MAX_OVERFULL_PT <n>])
#
# The manuals all share a layout: <module>/doc/<module>.tex, inputs scattered
# through <module>/doc, and the shared preamble and title page in common/doc.
# Given that, the module name is the only thing worth stating.
# ---------------------------------------------------------------------------
function(dyninst_add_manual _module)
  cmake_parse_arguments(M "" "ALLOW_WARNINGS;MAX_OVERFULL_PT" "" ${ARGN})
  set(_src "${PROJECT_SOURCE_DIR}/${_module}/doc")
  if(NOT EXISTS "${_src}/${_module}.tex")
    message(FATAL_ERROR "dyninst_add_manual: no ${_src}/${_module}.tex")
  endif()

  # Over-approximate rather than parse \input, \includegraphics and
  # \lstinputlisting: touching an unrelated file in one manual's directory
  # costs a rebuild of that manual alone, and nothing goes stale silently.
  set(_patterns "*.tex" "*.cc" "*.C" "*.pdf" "*.eps" "*.png" "*.dot")
  set(_deps "")
  foreach(_dir "${_src}" "${PROJECT_SOURCE_DIR}/common/doc")
    foreach(_p ${_patterns})
      file(GLOB_RECURSE _found CONFIGURE_DEPENDS "${_dir}/${_p}")
      list(APPEND _deps ${_found})
    endforeach()
  endforeach()
  list(REMOVE_DUPLICATES _deps)

  dyninst_add_latex_document(
    TARGET ${_module}.pdf
    SOURCE_DIR "${_src}"
    MAIN "${_module}.tex"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${_deps}
    ALLOW_WARNINGS "${M_ALLOW_WARNINGS}"
    MAX_OVERFULL_PT "${M_MAX_OVERFULL_PT}"
    INSTALL_DESTINATION "${DYNINST_DOCS_INSTALL_DIR}")
endfunction()

# ---------------------------------------------------------------------------
# dyninst_validate_listings(<module> SOURCES <file>... [USES <target>...])
#
# Compiles the sources a manual typesets with \lstinputlisting, so an example
# that stops matching the API it documents fails the manual's own build rather
# than the reader's first attempt.  <module>.pdf gains the check as a
# dependency; DYNINST_DOCS_VALIDATE_LISTINGS turns it off.
#
# USES names the library targets whose headers the examples include; it
# defaults to <module>.  Their usage requirements are borrowed rather than
# linked, because target_link_libraries would make the check depend on the
# library being *built* and "make docs" would compile all of Dyninst to
# typeset a manual.
#
# Borrowing keeps the distinction that matters.  A dependency found by
# find_package is an imported target, and CMake puts an imported target's
# headers behind -isystem; without that, Boost and oneTBB report warnings
# against these files that Dyninst never sees compiling its own sources.  The
# include directories have to be borrowed as $<TARGET_PROPERTY:...> rather
# than read with get_target_property: they hold $<BUILD_INTERFACE:a;b;c>, and
# any list operation splits that on its semicolons into fragments that are no
# longer a generator expression.
# ---------------------------------------------------------------------------
function(_dyninst_listing_targets _roots _out_local _out_imported)
  set(_seen "")
  set(_queue ${_roots})
  set(_local "")
  set(_imp "")

  while(_queue)
    list(POP_FRONT _queue _t)
    if(NOT TARGET ${_t} OR ${_t} IN_LIST _seen)
      continue()
    endif()
    list(APPEND _seen ${_t})

    get_target_property(_is_imported ${_t} IMPORTED)
    if(_is_imported)
      list(APPEND _imp ${_t})
    else()
      list(APPEND _local ${_t})
    endif()

    # Anything that is not a plain target name here -- a $<LINK_ONLY:...>
    # wrapper, a bare library path -- carries no usage requirements to
    # borrow, and the TARGET test above drops it.
    get_target_property(_l ${_t} INTERFACE_LINK_LIBRARIES)
    if(_l)
      list(APPEND _queue ${_l})
    endif()
  endwhile()

  set(${_out_local} "${_local}" PARENT_SCOPE)
  set(${_out_imported} "${_imp}" PARENT_SCOPE)
endfunction()

function(dyninst_validate_listings _module)
  cmake_parse_arguments(V "" "" "SOURCES;USES" ${ARGN})
  if(NOT V_SOURCES)
    message(FATAL_ERROR "dyninst_validate_listings: SOURCES is required")
  endif()
  if(NOT DYNINST_DOCS_VALIDATE_LISTINGS)
    return()
  endif()
  if(NOT V_USES)
    set(V_USES ${_module})
  endif()

  _dyninst_listing_targets("${V_USES}" _local _imported)

  set(_check ${_module}-doc-listings)
  add_library(${_check} OBJECT EXCLUDE_FROM_ALL ${V_SOURCES})
  set_target_properties(${_check} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_options(${_check} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})

  foreach(_t ${_local})
    target_include_directories(
      ${_check} PRIVATE $<TARGET_PROPERTY:${_t},INTERFACE_INCLUDE_DIRECTORIES>)
    target_compile_definitions(
      ${_check} PRIVATE $<TARGET_PROPERTY:${_t},INTERFACE_COMPILE_DEFINITIONS>)
  endforeach()

  foreach(_t ${_imported})
    target_include_directories(
      ${_check} SYSTEM PRIVATE $<TARGET_PROPERTY:${_t},INTERFACE_INCLUDE_DIRECTORIES>)
    target_compile_definitions(
      ${_check} PRIVATE $<TARGET_PROPERTY:${_t},INTERFACE_COMPILE_DEFINITIONS>)
  endforeach()

  if(TARGET ${_module}.pdf)
    add_dependencies(${_module}.pdf ${_check})
  endif()
endfunction()
