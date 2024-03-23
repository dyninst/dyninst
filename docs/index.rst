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

:ref:`sec:dataflow-intro`
   Trace the flow of data through a binary using the techniques of slicing, stack analysis,
   symbolic expansion and evaluation, and register liveness

:ref:`sec:dyncapi-intro`
  A C-like language for enabling rapid creation of Dyninst mutators without the need to specify API-level
  interactions.

:ref:`sec:dyninstapi-intro`
   An interface for instrumenting and working with binaries and processes

:ref:`sec:instructionapi-intro`
   Decode raw binary instructions into a platform-independent represention that provides
   a description of their semantics

:ref:`sec:parseapi-intro`
   Converts the machine code representation of a program, library, or code snippet into
   platform-independent abstractions such as instructions, basic blocks, functions, and loops

:ref:`sec:patchapi-intro`
   Instrument (insert code into) and modify a binary executable or library by manipulating
   the binary’s control flow graph (CFG)

:ref:`sec:proccontrolapi-intro`
   An API and library for controlling processes

:ref:`sec:stackwalk-intro`
   Collect and analyze stack traces

:ref:`sec:symtab-intro`
   A platform-independent representation of symbol tables, object file headers, and debug information

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

-------------------
Recent Publications
-------------------

- `Identifying and (Automatically) Remedying Performance Problems in CPU/GPU Applications <https://paradyn.org/papers/welton_autocorrect.pdf>`_

- `Parallelizing Binary Code Analysis <https://arxiv.org/pdf/2001.10621>`_

- `Binary Code Is Not Easy <https://paradyn.org/papers/Meng16MultiAuthor.pdf>`_

A complete list can be found at `<https://paradyn.org/publications>`_

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

   basics/building
   basics/importing
   basics/using
   examples/index

.. toctree::
   :caption: toolkit overviews
   :name: toolkit-overviews
   :hidden:
   :maxdepth: 3

   dataflowAPI/overview
   dyninstAPI/overview
   instructionAPI/overview
   parseAPI/overview
   patchAPI/overview
   proccontrol/overview
   stackwalk/overview
   symtabAPI/overview
   usertools/DynC/overview

.. toctree::
   :caption: toolkit api docs
   :name: apis
   :hidden:
   :maxdepth: 2

   common/public/API
   dataflowAPI/public/API
   dyninstAPI/public/API
   instructionAPI/public/API
   parseAPI/public/API
   patchAPI/public/API
   proccontrol/public/API
   stackwalk/public/API
   symtabAPI/public/API   

.. toctree::
   :caption: developer docs
   :name: dev-docs
   :hidden:
   :maxdepth: 3

   common/developer/API
   dataflowAPI/developer/API
   dwarf/developer/API
   dyninstAPI/developer/API
   dyninstAPI_RT/developer/API
   elf/developer/API
   instructionAPI/developer/API
   parseAPI/developer/API
   patchAPI/developer/API
   proccontrol/developer/API
   stackwalk/developer/API
   symtabAPI/developer/API

.. toctree::
   :caption: advanced
   :name: advanced
   :hidden:
   :maxdepth: 2
   
   advanced/optimizations
   advanced/pitfalls
