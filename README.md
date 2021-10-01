# Dyninst

## Branch states

| Branch                                  | Status        | Notes                                              |
| --------------------------------------- |:-------------:|:--------------------------------------------------:|
| master                                  | stable        | See below                                          |
| aarch32                                 | experimental  | Contact Ray Chen (rchen at cs dot umd dot edu)     |

## Notes

* Known issues should have open issues associated with them.
* ARMv8 (64 bit) support for dynamic instrumentation is experimental and incomplete.
  For more details about current supported functionality refer to [Dyninst Support for the ARMv8 (64 bit)](https://github.com/dyninst/dyninst/wiki/DyninstAPI-ARMv8-status).

## Build DyninstAPI and its subcomponents

### Install with Spack

```bash
$ spack install dyninst
```

### Build from source

1. Configure Dyninst with CMake

	```cmake /path/to/dyninst/source -DCMAKE_INSTALL_PREFIX=/path/to/installation```


	**NOTE:** If Dyninst builds TBB from source, see the [wiki](https://github.com/dyninst/dyninst/wiki/third-party-deps#tbb_correct_linking) for instructions on ensuring correct usage.

2. Build and install Dyninst in parallel

	```make install -jN```

If this does not work for you, please refer to the [Wiki](https://github.com/dyninst/dyninst/wiki) for detailed instructions. If you encounter any errors, see the [Building Dyninst](https://github.com/dyninst/dyninst/wiki/Building-Dyninst) or leave a [GitHub issue](https://github.com/dyninst/dyninst/issues).

### Docker Build

A [Dockerfile](Dockerfile) is provided that makes it easy to bring up a development environment.

```bash
$ docker build -t dyninst .
```

This works by way of installing with spack! You can then shell inside to interact with dyninst - there will be a spack environment
directory right in the root where you shell in:

```bash
$ docker run -it dyninst bash
```
```bash
# ls
spack.lock  spack.yaml
```

You can find the libraries in the spack enviroment:

```bash
.spack-env/view/lib/libdynC_API.so
.spack-env/view/lib/libdynC_API.so.11.0
.spack-env/view/lib/libdynC_API.so.11.0.1
.spack-env/view/lib/libdynDwarf.so
.spack-env/view/lib/libdynDwarf.so.11.0
.spack-env/view/lib/libdynDwarf.so.11.0.1
.spack-env/view/lib/libdynElf.so
.spack-env/view/lib/libdynElf.so.11.0
.spack-env/view/lib/libdynElf.so.11.0.1
.spack-env/view/lib/libdyninstAPI.so
.spack-env/view/lib/libdyninstAPI.so.11.0
.spack-env/view/lib/libdyninstAPI.so.11.0.1
.spack-env/view/lib/libdyninstAPI_RT.a
.spack-env/view/lib/libdyninstAPI_RT.so
.spack-env/view/lib/libdyninstAPI_RT.so.11.0
.spack-env/view/lib/libdyninstAPI_RT.so.11.0.1
```
Which you can then activate

```bash
$ . /opt/spack/share/spack/setup-env.sh
$ spack env activate .
```
```bash
# env | grep LD_LIBRARY_PATH
LD_LIBRARY_PATH=/opt/dyninst-env/.spack-env/view/lib64:/opt/dyninst-env/.spack-env/view/lib:/opt/view/lib:/opt/view/lib64
```
You could then make any changes to the source code at `/code`and then run spack install again to rebuild (without
needing to rebuild dependencies!)

```bash
$ spack install
```

And that's it! For a more interactive development environment, you can bind the present working directory with the
Dyninst source code to somewhere in the container (e.g., `/src`).

```bash
$ docker run -it -v $PWD:/src dyninst bash
```

If you do the same steps as above, you'll be in the activated environment:

```bash
$ . /opt/spack/share/spack/setup-env.sh
$ spack env activate .
```

And again navigate to the bound source code - this time the files are on your local machine, so you can edit
them locally and build in the container!

```bash
# This is bound to your host - edit files there
$ cd /src
```

And then again do:

```bash
$ spack install
```

Finally, if you just want a container for development with Dyninst ready to go, you
can use [ghcr.io/autamus/dyninst](https://github.com/orgs/autamus/packages/container/package/dyninst)
(view the page there for tags available). New tags are automatically detected when there is
a release to the repository here, and the container built and deployed. You can find the libraries
under the root `/opt/view` (and then lib, bin, share, etc.)

```bash
# ls /opt/view/lib/libd*
libdw-0.185.so              libdynC_API.so              libdynDwarf.so.11.0         libdynElf.so.11.0.1         libdyninstAPI_RT.a
libdw.a                     libdynC_API.so.11.0         libdynDwarf.so.11.0.1       libdyninstAPI.so            libdyninstAPI_RT.so
libdw.so                    libdynC_API.so.11.0.1       libdynElf.so                libdyninstAPI.so.11.0       libdyninstAPI_RT.so.11.0
libdw.so.1                  libdynDwarf.so              libdynElf.so.11.0           libdyninstAPI.so.11.0.1     libdyninstAPI_RT.so.11.0.1
root@89e4fbb001b5:/# ls /opt/view/lib/libsym*
libsymLite.so           libsymLite.so.11.0      libsymLite.so.11.0.1    libsymtabAPI.so         libsymtabAPI.so.11.0    libsymtabAPI.so.11.0.1
```

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
