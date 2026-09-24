#[=======================================================================[.rst:
FindGhostscript
---------------

Find Ghostscript, an interpreter for PostScript and PDF.

Ghostscript is used to measure a finished PDF: its ``bbox`` device reports
the extent of the ink on each page, which is how the manuals are checked for
content that runs off the paper.

The version matters to callers.  Ghostscript 10 replaced the PostScript
implementation of its PDF interpreter with one written in C, and the
``runpdfbegin`` and ``pdfgetpage`` operators that the older releases exposed
went with it; ``-dPDFINFO``, added in 9.56, is the replacement.  A caller that
reads a page's geometry has to pick its method accordingly.

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
