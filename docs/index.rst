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
   :caption: advanced
   :name: advanced
   :hidden:
   :maxdepth: 2
   
   advanced/optimizations
   advanced/pitfalls
