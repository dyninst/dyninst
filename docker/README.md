# Dyninst Containers

This is a testing set of Dockerfile for building a Dyninst base container.

## Strategy

For development, we want to be able to:

 1. Pull a container with dyninst ready to go (for development or dyninst or another library)
 2. Quickly update with our own source code without needing to build everything.

And for testing, we want to be able to:

 1. Have a set of base containers that make dependency preparation minimal
 2. Use those base containers to quickly test a PR's dyninst and report results
 
These are both achieved using a single workflow based on a set of base images with each containing a minimal OS environment and the package-provided versions of the various Dyninst dependencies. There is no source for Dyninst or any of its components. The development image is a small layer on top of a base and contains the Dyninst source and binaries. They are named by both the base OS and the hardware architecture. For example, the base container for Ubuntu 20.04 on x86_64 is `dyninst-amd64-base:ubuntu-20.04`. The corresponding development container is named `dyninst-amd64:ubuntu-20.04`.

## Usage

The Dockerfiles provided are intended for building the base and development containers from scratch. We don't recommend using these directly as images are provided as [packages](https://github.com/orgs/dyninst/packages).

### Build

A [Dockerfile](Dockerfile) is provided that makes it easy to bring up a development environment. The following builds the Ubuntu 20.04 images.

```bash
$ cd path/to/Dyninst/source

# Base image (Ubuntu 20.04 needs a newer elfutils)
$ docker build -f docker/Dockerfile.ubuntu -t dyninst-base:ubuntu-20.04 --build-arg version=20.04 --build-arg build_elfutils=yes .

# Development image
$ docker build -f docker/Dockerfile -t dyninst:ubuntu-20.04 --build-arg build_jobs=16 --build-arg base=dyninst-base:ubuntu-20.04 .
```

In the development container, Dyninst is installed in `/dyninst/install`.

### Develop

You can then shell inside to interact with dyninst:

```bash
$ docker run -it dyninst:ubuntu-20.04 bash
```

For a more interactive development environment, you can bind the present working directory with the
Dyninst source code bound to `/code` instead:

```bash
$ docker run -it -v $PWD:/code dyninst:ubuntu-20.04
```

And again navigate to the bound source code - this time the files are on your local machine, so you can edit
them locally and build in the container!

```bash
# This is bound to your host - edit files there
$ cd /code
```
