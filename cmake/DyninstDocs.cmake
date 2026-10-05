# Build the LaTeX manuals.  Nothing is written into the source tree.  The
# driver that runs pdflatex to a fixed point and checks what it produced is
# cmake/DyninstRunLaTeX.cmake.
#
# Targets
#   docs                    every manual
#   <module>.pdf            one manual
#   docs-install            install the manuals under CMAKE_INSTALL_DOCDIR
#   <module>-doc-listings   the example sources that manual typesets
#
# Options
#   DYNINST_BUILD_DOCS      ON requires pdflatex and puts the manuals in 'all'
#                           and 'install'; OFF builds them by name only, and
#                           a missing pdflatex is reported by the target
#   DYNINST_DOCS_VALIDATE_LISTINGS
#                           governs <module>-doc-listings, which are in 'all'
#                           whether or not DYNINST_BUILD_DOCS is set
#   DYNINST_DOCS_FORCE_VALIDATE
#                           require Ghostscript rather than skip the page
#                           measurement when it is absent
#   DYNINST_WARNINGS_AS_ERRORS, DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS
#                           govern the log check as they do the C++ build

include_guard(GLOBAL)
include(GNUInstallDirs)

find_package(LATEX COMPONENTS PDFLATEX)

if(DYNINST_BUILD_DOCS AND NOT LATEX_PDFLATEX_FOUND)
  message(
    FATAL_ERROR
      "DYNINST_BUILD_DOCS is ON but pdflatex was not found.\n"
      "  Install a LaTeX distribution, or configure with "
      "-DDYNINST_BUILD_DOCS=OFF to build the manuals on demand only.")
endif()

find_package(Ghostscript)

if(DYNINST_DOCS_FORCE_VALIDATE AND NOT Ghostscript_FOUND)
  message(
    FATAL_ERROR
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
  set(_one TARGET SOURCE_DIR MAIN INSTALL_DESTINATION ALLOW_WARNINGS MAX_OVERFULL_PT)
  set(_many DEPENDS)
  cmake_parse_arguments(LTX "${_opts}" "${_one}" "${_many}" ${ARGN})

  foreach(_r TARGET SOURCE_DIR MAIN)
    if(NOT LTX_${_r})
      message(FATAL_ERROR "dyninst_add_latex_document: ${_r} is required")
    endif()
  endforeach()
  if(LTX_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR "dyninst_add_latex_document: unrecognised: ${LTX_UNPARSED_ARGUMENTS}")
  endif()

  # Beside the CMakeLists.txt that asked for it.  The pass cap is not settable
  # here either; DyninstRunLaTeX owns it, so there is one default.
  set(_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

  get_filename_component(_stem "${LTX_MAIN}" NAME_WE)
  set(_pdf "${_out_dir}/${_stem}.pdf")

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
    BYPRODUCTS "${_out_dir}/${_stem}.aux" "${_out_dir}/${_stem}.log"
               "${_out_dir}/${_stem}.toc" "${_out_dir}/${_stem}.out"
    COMMAND
      ${CMAKE_COMMAND} -DPDFLATEX=${PDFLATEX_COMPILER} -DSOURCE_DIR=${LTX_SOURCE_DIR}
      -DOUTPUT_DIR=${_out_dir} -DMAIN=${LTX_MAIN} -DALLOW_WARNINGS=${LTX_ALLOW_WARNINGS}
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

    # install(FILES ... OPTIONAL) is silent about a PDF that was never built,
    # so record what a complete install holds; docs/CMakeLists.txt writes the
    # list out for CI.  Keep this beside the install() rule it describes.
    set_property(GLOBAL APPEND PROPERTY DYNINST_DOCS_INSTALLED_FILES "${_stem}.pdf")
  endif()
endfunction()

# ---------------------------------------------------------------------------
# dyninst_add_manual(<module> [ALLOW_WARNINGS <regex>] [MAX_OVERFULL_PT <n>])
#
# The manuals all share a layout: docs/<module>/manual-latex/<module>.tex,
# with its inputs beside it and the shared preamble and title page in
# docs/common/manual-latex.  The module name is all that is needed.
# ---------------------------------------------------------------------------
function(dyninst_add_manual _module)
  cmake_parse_arguments(M "" "ALLOW_WARNINGS;MAX_OVERFULL_PT" "" ${ARGN})
  set(_src "${PROJECT_SOURCE_DIR}/docs/${_module}/manual-latex")
  if(NOT EXISTS "${_src}/${_module}.tex")
    message(FATAL_ERROR "dyninst_add_manual: no ${_src}/${_module}.tex")
  endif()

  # Over-approximate rather than parse \input, \includegraphics and
  # \lstinputlisting: touching an unrelated file in one manual's directory
  # costs a rebuild of that manual alone, and nothing goes stale silently.
  set(_patterns
      "*.tex"
      "*.c"
      "*.cc"
      "*.cpp"
      "*.C"
      "*.h"
      "*.pdf"
      "*.eps"
      "*.png"
      "*.dot")
  set(_deps "")
  foreach(_dir "${_src}" "${PROJECT_SOURCE_DIR}/docs/common/manual-latex")
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
    DEPENDS ${_deps}
    ALLOW_WARNINGS "${M_ALLOW_WARNINGS}"
    MAX_OVERFULL_PT "${M_MAX_OVERFULL_PT}"
    INSTALL_DESTINATION "${DYNINST_DOCS_INSTALL_DIR}")
endfunction()

# ---------------------------------------------------------------------------
# dyninst_validate_listings(<module> SOURCES <file>... [USES <target>...])
#
# Compiles the sources a manual typesets, so an example that stops matching
# the API it documents fails a build rather than the reader's first attempt.
# In 'all' whether or not DYNINST_BUILD_DOCS is set, because the change that
# breaks an example is a change to the library; <module>.pdf depends on it too,
# and DYNINST_DOCS_VALIDATE_LISTINGS turns it off.
#
# USES names the library targets whose headers the examples include, default
# <module>.  Borrow their usage requirements; do not link them, or "make docs"
# builds all of Dyninst to typeset a manual.
#
# Borrow include directories as $<TARGET_PROPERTY:...>, never with
# get_target_property: the value holds $<BUILD_INTERFACE:a;b;c>, and any list
# operation splits it on those semicolons into fragments that are no longer a
# generator expression.  Borrowing also keeps an imported target's headers
# behind -isystem, without which Boost and oneTBB warn about these files.
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

  set(${_out_local}
      "${_local}"
      PARENT_SCOPE)
  set(${_out_imported}
      "${_imp}"
      PARENT_SCOPE)
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
  # Deliberately in 'all': the change that breaks an example is made by
  # someone with no reason to build a manual, so the check has to reach them.
  # Nine small object libraries, and no LaTeX involved.
  add_library(${_check} OBJECT ${V_SOURCES})
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
