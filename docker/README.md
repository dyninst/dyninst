# Dyninst Containers

This is a testing set of Dockerfile for building a Dyninst base container.

## Strategy

For development, we want to be able to:

 1. Pull a container with dyninst ready to go (for development or dyninst or another library)
 2. Quickly update with our own source code without needing to build everything.

And for testing, we want to be able to:

 1. Have a set of base containers that make dependency preparation minimal
 2. Use those base containers to quickly test a PR's dyninst and report results
 
To do this we will have two workflows:

### Base Containers

The [base-containers.yaml](../.github/workflows/base-containers.yaml) workflow will build updated base
containers nightly or at an appropriate frequency. Right now we are providing one container build with a reasonable set of dependencies
and dyninst, and this is done by way of installing via spack, and setting versions
in an environment via build args. The current workflow, although there is just one container,
done by using a matrix so in the future when additional build configurations are added,
we can just extend the matrix. When this time comes, we can have a simple script (e.g., Python) 
that generates this same matrix, and then pipes it into the GitHub workflow to still use as 
build args in the Dockerfile. It's setup right now to still use this build arg -> environment -> matrix
strategy so the transition is easier.

### Test Container

The [test-dyninst.yaml](../.github/workflows/test-dyninst.yaml) workflow
has it's own Dockerfile, [Dockerfile.test](docker/Dockerfile.test) that starts with a base container (ready to go with
dependencies, Dyninst compiler, and the testsuite compiler) and then adds Dyninst 
and an updated testsuite to it, compiles changes, and then runs tests. Given that the base
containers are pre-built, the entire workflow takes only a few minutes.

## Usage

While most of this is done via recipes in CI, here is how to interact with the repository locally.

### Build

A [Dockerfile](Dockerfile) is provided that makes it easy to bring up a development environment.

```bash
$ docker build -t dyninst -f Dockerfile ../
```

### Develop

You can then shell inside to interact with dyninst - there will be a spack environment
directory right in the root where you shell in:

```bash
$ docker run -it dyninst bash
```
```bash
# ls
spack.lock  spack.yaml
```

This lock and spack yaml defines a spack envirironment, which is generated during the container build. If we inspect closer, we would see:

```yaml
# This is a Spack Environment file.
#
# It describes a set of packages to be installed, along with
# configuration settings.
spack:
  # add package specs to the `specs` list
  specs: [boost@1.73.0, elfutils@0.186, libiberty@2.33.1, intel-tbb@2020.2, perl@5.32.1]
  view: true
  concretization: together
  develop:
    dyninst:
      path: /code
      spec: dyninst@master
```

Which includes a set of depdencies and versions we want to build. If you are just running a container as a developer, you are ready to go,
because you can find all the dyninst libraries in the spack enviroment:

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

And here is an example of what activation does to the `LD_LIBRARY_PATH`:

```bash
# env | grep LD_LIBRARY_PATH
LD_LIBRARY_PATH=/opt/dyninst-env/.spack-env/view/lib64:/opt/dyninst-env/.spack-env/view/lib:/opt/view/lib:/opt/view/lib64
```
You could then make any changes to the source code at `/code` (even if you've bound from your local machine to open with your editor of choice) and then run spack install again to rebuild (without needing to rebuild dependencies!)

```bash
$ spack install --reuse
```

Note that we've added "reuse" to tell spack to be as flexible as it can to reuse what it has.
And that's it! For a more interactive development environment, you can bind the present working directory with the
Dyninst source code bound to `/code` instead:

```bash
$ docker run -it -v $PWD:/code dyninst
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
$ cd /code
```

And then again do:

```bash
$ spack install --reuse
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

## Test

To build the test container, you'll want to use [Dockerfile.test](docker/Dockerfile.test). If you want to use
the base dyninst container from GitHub packages, leave out build args (it will default to this). 

```bash
$ cd docker/
$ docker build -f Dockerfile.test -t dyninst-test ../
```

Otherwise, if you want to use a container you just built, specify that as the build arg `dyninst_base`:

```bash
$ docker build -f Dockerfile.test --build-arg dyninst_base=dyninst -t dyninst-test ../
```

The tests are added and run during build, so if they fail inside the container, the container build will fail
(and trigger any CI building it to fail).
