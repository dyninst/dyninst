================
Building Dyninst
================

Docker Containers
-----------------

Containers are provided that can be used for Dyninst development (e.g., make changes to Dyninst and quickly rebuild it)
or for development of your own tools (e.g., have a container ready to go with Dyninst). All containers are available
through the Github package repository: https://github.com/orgs/dyninst/packages.


Install with Spack
------------------

``spack install dyninst``

Build from source
-----------------

1. Configure Dyninst with CMake

   ``cmake /path/to/dyninst/source -DCMAKE_INSTALL_PREFIX=/path/to/installation``

2. Build and install Dyninst in parallel

   ``cmake --install . --parallel``
