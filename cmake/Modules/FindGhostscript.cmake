#[=======================================================================[.rst:
FindGhostscript
---------------

Find Ghostscript, an interpreter for PostScript and PDF.

Ghostscript is used to measure a finished PDF: its ``bbox`` device reports
the extent of the ink on each page, which is how the manuals are checked for
content that runs off the paper.

The version is reported because Ghostscript's PDF interpreter changed
incompatibly: version 10 replaced the PostScript implementation with one
written in C, taking the ``runpdfbegin`` and ``pdfgetpage`` operators with it,
and ``-dPDFINFO`` -- the replacement -- exists only from 9.56.  A caller that
needs a page's geometry must choose between them, and can ask for a version
here.  Dyninst does not: it takes the page size from the LaTeX log and asks
Ghostscript only for the ``bbox`` device, which every release accepts.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Ghostscript_EXECUTABLE``
  the Ghostscript interpreter
``Ghostscript_FOUND``
  If false, do not try to use Ghostscript.
``Ghostscript_VERSION``
  the version of Ghostscript found

#]=======================================================================]

# gs everywhere but Windows, where the console builds carry their word size.
find_program(Ghostscript_EXECUTABLE NAMES gs gswin64c gswin32c)

if(Ghostscript_EXECUTABLE)
  execute_process(
    COMMAND "${Ghostscript_EXECUTABLE}" --version
    OUTPUT_VARIABLE _gs_version
    ERROR_QUIET
    RESULT_VARIABLE _gs_rc
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(_gs_rc EQUAL 0 AND _gs_version MATCHES "^[0-9]+\\.[0-9]+")
    set(Ghostscript_VERSION "${_gs_version}")
  endif()
  unset(_gs_version)
  unset(_gs_rc)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Ghostscript
  FOUND_VAR Ghostscript_FOUND
  REQUIRED_VARS Ghostscript_EXECUTABLE
  VERSION_VAR Ghostscript_VERSION)

mark_as_advanced(Ghostscript_EXECUTABLE)
