# Dyninst

## Notes

* Known issues should have open issues associated with them.
* ARMv8 (64 bit) support for dynamic instrumentation is experimental and incomplete.
  For more details about current supported functionality refer to [Dyninst Support for the ARMv8 (64 bit)](https://github.com/dyninst/dyninst/wiki/DyninstAPI-ARMv8-status).

## Build DyninstAPI and its subcomponents

### Docker Containers

Containers are provided that can be used for Dyninst development (e.g., make changes to Dyninst and quickly rebuild it)
or for development of your own tools (e.g., have a container ready to go with Dyninst). Links will be added
here when the containers are pushed to the Dyninst associated package registries. Instructions for usage
and building locally are provided in the [docker](docker) directory.


### Install with Spack

```spack install dyninst```

### Build from source

1. Configure Dyninst with CMake

	```cmake /path/to/dyninst/source -DCMAKE_INSTALL_PREFIX=/path/to/installation```

2. Build and install Dyninst in parallel

	```make install -jN```

If this does not work for you, please refer to the [Wiki](https://github.com/dyninst/dyninst/wiki) for detailed instructions. If you encounter any errors, see the [Building Dyninst](https://github.com/dyninst/dyninst/wiki/Building-Dyninst) or leave a [GitHub issue](https://github.com/dyninst/dyninst/issues).

## Known Issues

* Windows 64-bit mode is not yet supported

* Windows rewriter mode is not yet supported

* Exceptions in relocated code will not be caught

* Linux rewriter mode for 32-bit, statically linked binaries does not support binaries with .plt, .rel, or .rela
sections.

* Callbacks at thread or process exit that stop the process will deadlock when a SIGSEGV occurs on a thread other than
the main thread of a process

* Stackwalker is fragile on Windows

* Parsing a binary with no functions (typically a single object file) will crash at CodeObject destruction time.
