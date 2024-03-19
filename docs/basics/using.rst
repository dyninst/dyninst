.. _`sec-importing`:

Importing Dyninst
#################

Dyninst supports the standard mechanism of consuming a CMake project.

.. code:: CMake

  find_package(Dyninst 13.0.0 REQUIRED COMPONENTS dyninstAPI)

  target_link_libraries(your_library Dyninst::dyninstAPI)

The toolkit names are dyninstAPI, instructionAPI, parseAPI, patchAPI, pcontrol, stackwalk, and symtabAPI. Dependent
tookits are automatically imported. For example, dyninstAPI imports instructionAPI.

ABI compatibility is only guaranteed for between major versions, so the package version uses
`SameMajorVersion <https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-version-file>`_
for version constraints.

Dyninst also follows the `Transitive Usage Requirements <https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#transitive-usage-requirements>`_.
If you build Dyninst using a dependency that is not in a system path, then you must pass that argument to your
project when building against Dyninst. For example, if you build Dyninst with a special TBB


.. code:: shell

  $ cmake /path/to/Dyninst -DTBB_ROOT_DIR=/path/to/intel-tbb

then you need to pass that to your project, as well.

.. code:: shell

  $ cmake /path/to/your/project -DTBB_ROOT_DIR=/path/to/intel-tbb

If your project uses TBB, then this also ensures that your project and Dyninst use the same version
of TBB.
