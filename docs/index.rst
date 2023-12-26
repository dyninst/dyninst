.. _manual-main:

.. image:: https://img.shields.io/github/stars/dyninst/dyninst?style=social
    :alt: GitHub stars
    :target: https://github.com/dyninst/dyninst/stargazers

=======
Dyninst
=======

.. epigraph::

   *Part of the* `Paradyn <http://www.paradyn.org>`_ *project*

Dyninst is a collection of libraries for performing binary instrumentation, analysis, and
modification. These libraries are assembled into a collection of toolkits that allow users
to more effectively use different aspects of binary analysis for building their own tools.

:ref:`sec:dyncapi-intro`
  A C-like language for enabling rapid creation of Dyninst mutators without the need to specify API-level
  interactions.

:ref:`sec:dyninstapi-intro`
   An interface for instrumenting and working with binaries and processes

:ref:`sec:instruction-intro`
   Decode raw binary instructions into a platform-independent represention that provides
   a description of their semantics

:ref:`sec:parseapi-intro`
   Converts the machine code representation of a program, library, or code snippet into
   platform-independent abstractions such as instructions, basic blocks, functions, and loops

:ref:`sec-patchapi-intro`
   Instrument (insert code into) and modify a binary executable or library by manipulating
   the binary’s control flow graph (CFG)

:ref:`sec:proccontrolapi-intro`
   An API and library for controlling processes

.. _main-support:

-------
Support
-------

* For **bugs and feature requests**, please use the `issue tracker <https://github.com/dyninst/dyninst/issues>`_.
* For **contributions**, visit us on `Github <https://github.com/dyninst/dyninst>`_.

---------
Resources
---------

`GitHub Repository <https://github.com/dyninst/dyninst>`_
    The code on GitHub.

------------
Developed by
------------

|     Paradyn Parallel Performance Tools

|     Computer Science Department
|     University of Wisconsin-Madison
|     Madison, WI 53706

|     Computer Science Department
|     University of Maryland
|     College Park, MD 20742

.. toctree::
   :caption: getting started
   :name: basics
   :hidden:
   :maxdepth: 1

   basics/overview
   basics/building
   basics/using
   basics/first_mutator

.. toctree::
   :caption: toolkit overviews
   :name: toolkit-overviews
   :hidden:
   :maxdepth: 3

   dyninstAPI/overview
   instructionAPI/overview
   parseAPI/overview
   patchAPI/overview
   proccontrol/overview
   usertools/DynC/overview

.. toctree::
   :caption: examples
   :name: examples
   :hidden:
   :maxdepth: 2

   examples/binary_analysis
   examples/cfg
   examples/function_disassembly
   examples/instrument_function
   examples/memory_access
   examples/retee

.. toctree::
   :caption: toolkit api docs
   :name: apis
   :hidden:
   :maxdepth: 2

   dyninstAPI/public/API
   instructionAPI/public/API
   parseAPI/public/API
   patchAPI/public/API
   proccontrol/public/API

.. toctree::
   :caption: developer docs
   :name: dev-docs
   :hidden:
   :maxdepth: 3

   dyninstAPI/developer/API
   instructionAPI/developer/API
   parseAPI/developer/API
   patchAPI/developer/API
   proccontrol/developer/API

.. toctree::
   :caption: advanced
   :name: advanced
   :hidden:
   :maxdepth: 2
   
   advanced/optimizations
   advanced/pitfalls
