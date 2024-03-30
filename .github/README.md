# GitHub CI testing

Reading through the [overview](https://resources.github.com/devops/ci-cd/) of GitHub CI/CD is highly recommended. A [primer](https://learnxinyminutes.com/docs/yaml/) on yaml may also be helpful. A brief overview of actions and workflows is provided at the bottom of this page.

## Actions

### Build Dyninst ([actions/build](actions/build/action.yaml))

*Builds a single configuration of Dyninst on one of the pre-built containers in the Dyninst container repo.*

This just forwards its arguments on to one of the OS-specific build actions.

**Inputs**
- **os**
  The operating system to use
- **extra-cmake-flags**
  Additional flags directly passed to CMake.
- **compiler**
  The compiler to use.
- **compiler-version**
  The compiler's version.
- **build-type**
  CMake build type to use. Default is `RELWITHDEBINFO`.
- **install-dir**
  Where to install Dyninst.
- **src-dir**
  Location of Dyninst source to build.
- **num-jobs**
  Number of CMake build jobs (i.e., `cmake --build --parallel N`).
- **extra-libs**
  Additional libraries to install via the package manager.


### Build Fedora ([actions/build-fedora](actions/build-fedora/action.yaml))

*Builds a single configuration of Dyninst on one of the pre-built Fedora containers in the Dyninst container repo.*

Only one version of gcc or clang is made available. This is because it is very  difficult to get multiple versions of any compiler installed simultaneously on Fedora.

**Inputs**
- **extra-cmake-flags**
  Additional flags directly passed to CMake
- **compiler**
  The compiler to use.
- **compiler-version**
  The compiler's version.
- **build-type**
  CMake build type to use. CMake build type to use. Default is `RELWITHDEBINFO`.
- **install-dir**
  Where to install Dyninst.
- **src-dir**
  Location of Dyninst source to build.
- **num-jobs**
  Number of CMake build jobs (i.e., `cmake --build --parallel N`).
- **extra-libs**
  Additional libraries to install via the package manager.

### Build Ubuntu ([actions/build-ubuntu](actions/build-ubuntu/action.yaml))

*Builds a single configuration of Dyninst on one of the pre-built Ubuntu containers in the Dyninst container repo.*

Clang-specific libraries are installed automatically. For example, `compiler: 'clang'` and `compiler-version: 15` will install `libomp-15-dev` and remove all other versions of `libomp`.

**Inputs**
- **extra-cmake-flags**
  Additional flags directly passed to CMake.
- **compiler**
  The compiler to use.
- **compiler-version**
  The compiler's version.
- **build-type**
  CMake build type to use. Default is `RELWITHDEBINFO`.
- **install-dir**
  Where to install Dyninst.
- **src-dir**
  Location of Dyninst source to build.
- **num-jobs**
  Number of CMake build jobs (i.e., `cmake --build --parallel N`).
- **extra-libs**
  Additional libraries to install via the package manager.

### CMake build types ([actions/build-types](actions/build-types/action.yaml))

*Returns the CMake build types used in CI.*

**Outputs**
- **all**
  All build types used by Dyninst as a JSON array of strings.

### Operating systems ([actions/os-versions](actions/os-versions/action.yaml))

*Returns the operating systems Dyninst is tested on.*

**outputs**
- **all**
  All OS names as a JSON array of strings.
- **latest**
  The most-recent supported version of each OS as a JSON array of strings.

---

## Workflows

### Build options ([workflows/build-opts](workflows/build-opts.yaml))

*Builds Dyninst with different values for each of its CMake options.*

This ensures that Dyninst builds on all supported platforms and compilers in non-standard configurations (e.g., disabling OpenMP), with different linkers, and different C++ standard libraries.

**when**: Every Monday at 3AM CST.

### CMake formatting ([workflows/cmake-formatting.yaml](workflows/cmake-formatting.yaml))

*Ensures all CMake files are correctly formatted using `cmake-format` and the rules in `.cmake-format.yaml` in the root of the project.*

**when**: on pull requests to the master branch when a CMake file is changed.

### Build with many compilers ([workflows/compiler-multibuild](workflows/compiler-multibuild.yaml))

*Builds Dyninst with all supported versions of gcc and clang available on Ubuntu, the built-in versions of gcc and clang on Fedora, and the supported C++ standards.*

This is a complex workflow that generates dozens of jobs. It is complicated by the fact that Fedora makes it very difficult to install multiple compiler versions simultaneously. In particular, it only offers one version of gcc and clang via 'yum'. Conversely, Ubuntu allows for arbitrary numbers and versions of compilers. This makes the parameter space large enough that it is easier to generate it using a python [script](scripts/compiler_configs.py).

**when**: Every Monday at 3AM CST.

### Dyninst consumers ([workflows/consumers](workflows/consumers.yaml))

*Builds the latest version of applications that consume Dyninst*

All consumers are built against the HEAD of Dyninst's master branch using the latest version of each supported operating system. Some consumers are more easily built with Spack and some are easier to build from source.

Spack builds:
- [HPCToolkit](http://hpctoolkit.org/) development branch

  Substantial efforts are undertaken to reduce the number of libraries built by spack. This is because there are often timeouts in fetching sources when running from GitHub CI machines.

- [omnitrace](https://amdresearch.github.io/omnitrace) development branch


From-source builds:
- [Must](https://www.i12.rwth-aachen.de/go/id/nrbe) latest release

  They don't provide a public repository, so the tested version will need to be updated manually.
  
- [systemtap](https://sourceware.org/systemtap/) development branch
- [STAT](https://hpc.llnl.gov/software/development-environment-software/stat-stack-trace-analysis-tool) development branch
- [TAU](https://www.cs.uoregon.edu/research/tau/home.php) development branch

**when**: Every Monday at 3AM CST.

### Third-party dependency versions ([workflows/dependency-version](workflows/dependency-version.yaml))

*Checks that the minimum dependency versions found in the various CMake files match the expected values. This ensures we synchronize versions across containers and workflows.*

**when**: on pull request to the master branch when a CMake file or docker/dependencies.versions is changed.

### ABI breaks with libabigail ([workflows/libabigail](workflows/libabigail.yaml))

*Checks for ABI breaks between the pull request and the latest release of Dyninst*

This runs libabigail's `abidiff` utility to compare the current changes against the last release. Only removed or modified functions in public interfaces are reported. The check is not run if the pull request is already marked as an ABI-breaking change.

**when**: on pull request to the master branch when source code changes.

### Pull request checks ([workflows/pr-tests](workflows/pr-tests.yaml))

*Checks run on every pull request that changes source code*

Build Dyninst, build the test suite, build examples, and build and run external tests.

Each step except building the test suite uses all supported OSes and their default versions of gcc and clang. The test suite only builds with gcc. It can't be executed under GitHub CI because they do not allow using ptrace.

**when**: on pull request to the master branch when source code changes.

### Purge old workflow runs ([workflows/purge-workflows](workflows/purge-workflows.yaml)

*Removes all workflow runs older than two weeks.*

GitHub doesn't provide a retention policy for workflow run, so we purge them once per month. This is not strictly necessary because open-source projects do not have file system usage limits. However, it's nice to have the workflow tab uncluttered.

**when**: The 1st of every month.

### Spack build ([workflows/spack-build](workflows/spack-build.yaml))

*Checks that Dyninst builds in the latest version of Spack*

This runs on the most-recent version of each supported OS.

**when**: Every Sunday at 3AM CST.

### Parse system libraries ([workflows/system-libs](workflows/system-libs.yaml))

*Checks that system libraries can be parsed using ParseAPI*

This runs [simpleParser](https://github.com/dyninst/external-tests/blob/master/parseAPI/simpleParser.cpp) from the external-tests repository on every shared library in `/lib{64}` in the container for each supported operating system.

**when**: Every Monday at 1AM CST.

### Unit tests ([workflows/unit-tests](workflows/unit-tests.yaml))

*Build and execute the unit tests for Dyninst*

This is separate from the [pr-tests](workflows/pr-tests.yaml) workflow because the unit tests need Dyninst to be built with `DYNINST_EXPORT_ALL` enabled to expose hidden symbols in internal classes that are tested here.

The unit tests are stored in the [dyninst/unit-tests](https://github.com/dyninst/unit-tests) repository.

**when**: on pull request to the master branch when source code changes.


## Container maintenance

Many different containers (aka packages) are used for both testing and conveniences for users. These require periodic updating to ensure they stay current with both the changes in Dyninst as well as the underlying Linux distribution.

Currently, only AMD64 packages are provided.

### Development containers ([workflows/dev-containers](workflows/dev-containers.yaml))

*Builds and deploys development containers for all supported operating systems.*

A 'development' container is an environment built atop the corresponding base container with both the latest Dyninst source code and a an installation of it located in /dyninst/src and /dyninst/install, respectively. The installation is built using the distributions default version of gcc.

Uses docker/Dockerfile to build and deploy the images.

**when**: on push to the master branch or on dispatch.

### Base containers ([workflows/base-containers](workflows/base-containers.yaml))

*Builds and deploys base containers for all supported operating systems.*

A 'base' container is an environment with the distribution's default versions of gcc, clang, git, cmake, static glibc, and all of Dyninst's dependencies installed. A python interpreter is purposefully not installed to reduce image size. It uses the respective `Dockerfile.<OS>` under the docker/directory to build the images.

When the version of a dependency provided by the distribution is older than the version in docker/dependencies.versions, then it is necessary to build that dependency from source. See 'docker/Dockerfile.ubuntu' for an example.

### Purge unused container images ([workflows/purge-containers](workflows/purge-containers.yaml)

*Removes all but the most-recent base and dev containers (aka. GitHub packages)*

GitHub doesn't provide a retention policy for packages, so we purge them once per month. The development containers are built every time a pull request is merged, but only the most recent one is really of any use since the previous ones contain outdated code. Only tagged containers such as 'latest' or 'v13.0.0' are kept.

**when**: on manual dispatch.

### Build and deploy release containers ([workflows/release-containers](workflows/release-containers.yaml)

*Builds and deploys release containers for all supported operating systems*

These are development containers with a fixed version of Dyninst.

Currently, this is only set up to be run manually. In the future, it can be run [automatically](https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#release) when a release is created in GitHub.

**when**: on manual dispatch.

### Refresh base and development containers ([workflows/release-containers](workflows/release-containers.yaml)

*Refreshes base containers for all supported operating systems*

Linux distributions periodically update their public containers- even the ones for a fixed version. It's a good idea to track these changes for the Dyninst packages, so this job is used to automatically account for this. The steps are

1. [Purge](workflows/purge-containers.yaml) all but the most-recent base and development containers
2. Build and deploy the [base](workflows/base-containers.yaml) containers using the latest distro images
3. Build and deploy the [development](workflows/dev-containers.yaml) containers.

With this sequence, we are guaranteed to have two versions of the base containers based on the most-recent two versions of the distro images (e.g., Ubuntu-24.04 from January and February). This isn't strictly necessary, but it might be useful for debugging purposes. The latest development containers are always based on the latest base containers.

Github scheduled jobs use cron which doesn't allow specifying 'the last Wednesday of the month', so the job runs on the 25th. Except for February, that should always be on a weekday in the last week of the month. The exact timing isn't critical because the old images aren't used in any of the workflows, but it should happen at a time when any failures can be detected and handled quickly.

**when**: 25th of every month.


---
# Updating workflows

## Adding a new OS version

1. Add it to the `names` array in the OS [action](actions/os-versions/action.yaml). If it's newer than the rest, then update the `names` array in the `latest` step.

2. Update the config [script](scripts/compiler_configs.py).

## Adding a new OS

TODO.

## Adding a new compiler version

TODO.

## Adding a new compiler

TODO.

## Adding a new test

TODO.

## Updating a dependency version

TODO.

## Adding a new dependency

TODO.

---

# Overview of GitHub CI features

## Using actions

Github [actions](https://docs.github.com/en/actions) are a collection of steps that can either be run on their own or inserted into another workflow or action. Any outputs can then be referenced by subsequent steps in the same job. For example,

**actions/hello/action.yaml**

```
name: Say hello!
inputs:
  name:
    required: true
outputs:
  hello-string:
    value: ${{ steps.say.outputs.an_arbitrary_string }}
runs:
  using: "composite"
  steps:
    - id: say
      shell: bash
      run: |
        echo an_arbitrary_string="Hello, ${{ inputs.name }}!" >> $GITHUB_OUTPUT
```

**workflows/hello.yaml**

```
name: Hello

on:
  pull_request:
     branches:
        - master

jobs:
  say-hello:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - id: hello
        uses: ./.github/actions/hello
        with:
          name: "Bob"
      - run: |
          echo ${{ steps.hello.outputs.hello-string }}
```

## Using workflows

Workflows are similar to actions, but return their values (if any) via artifacts rather than as
values in $GITHUB_OUTPUT. See the [docs](https://docs.github.com/en/actions/using-workflows/about-workflows) for details.

### Matrix strategy

The workflows here make extensive use of the matrix [strategy](https://docs.github.com/en/actions/using-jobs/using-a-matrix-for-your-jobs) to generate multiple jobs. It is important to note that the inputs to a matrix must either be literals or come from the output of another job.

The `say-hello` job from the `hello` workflow above can be modified to address multiple names.

```
jobs:
  say-hello:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        name: ["Alice", "Bob", "Eve"]
    steps:
      - uses: actions/checkout@v4
      - id: hello
        uses: ./.github/actions/hello
        with:
          name: ${{ matrix.name }}
      - run: |
          echo ${{ steps.hello.outputs.hello-string }}
```

This will generate three jobs, each printing one name.


When multiple features are provided to a matrix, it creates jobs spanning the Cartesian product of the values. The below builds Dyninst using the latest version of each supported OS and each build type. Currently, this creates six jobs (e.g., os='fedora-39', bt='RELEASE').

```
jobs:

  get-build-types:
    runs-on: ubuntu-latest
    outputs:
      all:  ${{ steps.build-type.outputs.all }}
    steps:
      - name: Checkout Dyninst
        uses: actions/checkout@v4

      - id: build-type
        uses: ./.github/actions/build-types

  get-oses:
    runs-on: ubuntu-latest
    outputs:
      latest: ${{ steps.all.outputs.latest }}
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - id: all
        uses: ./.github/actions/os-versions

  build:
    runs-on: ubuntu-latest
    needs: [get-oses, get-build-types]
    strategy:
      matrix:
        os: ${{ fromJson(needs.get-oses.outputs.latest) }}
        bt: ${{ fromJson(needs.get-build-types.outputs.all }}
    steps:
    - name: Checkout Dyninst
      uses: actions/checkout@v4

    - name: Build Dyninst
      uses: ./.github/actions/build
      with:
        os: ${{ matrix.os }}
        compiler: "gcc"
        build-type: ${{ matrix.bt }}
```
