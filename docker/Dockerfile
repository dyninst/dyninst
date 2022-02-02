ARG ubuntu_version=20.04
FROM ubuntu:${ubuntu_version}

# Build the Dockerfile in this directory, context one level up
# docker build -t dyninst -f Dockerfile ../

LABEL maintainer="@hainest,@vsoch"

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Los_Angeles

# We can use build args to populate the specific versions of dependencies (with defaults here)
ARG BOOST_VERSION=1.73.0
ARG ELFUTILS_VERSION=0.186
ARG LIBIBERTY_VERSION=2.33.1
ARG INTELTBB_VERSION=2020.2
ARG PERL_VERSION=5.32.1

# Sort of separate from dyninst (but used to build/test)
ARG CMAKE_VERSION=3.21.2
ENV CMAKE_VERSION=${CMAKE_VERSION}

# Set the branch name for spack to use (should be master even for PR)
ARG DYNINST_BRANCH=master
ENV DYNINST_BRANCH=${DYNINST_BRANCH}

# Note that perl 5.30.0 is provided by ubuntu

# Args need to be passed into envars to be used in RUN
ENV BOOST_VERSION=${BOOST_VERSION}
ENV ELFUTILS_VERSION=${ELFUTILS_VERSION}
ENV LIBIBERTY_VERSION=${LIBIBERTY_VERSION}
ENV INTELTBB_VERSION=${INTELTBB_VERSION}
ENV PERL_VERSION=${PERL_VERSION}

RUN apt-get -qq update && \
    apt-get -qq install -fy tzdata && \
    apt-get -qq install -y --no-install-recommends \
      build-essential \
      bzip2 \
      ca-certificates \
      curl \
      dh-autoreconf \
      git \
      gnupg2 \
      lcov \
      libssl-dev \
      ninja-build \
      pkg-config \
      python-dev \ 
      python3-pip \
      sudo \
      valgrind \
      vim \
      wget \
      xsltproc

# Install Clingo for Spack
RUN python3 -m pip install --upgrade pip && \
    python3 -m pip install clingo && \
    dpkg-reconfigure tzdata

# Update gcc to 11.1.1 (otherwise we'd have 9.3.0)
RUN apt-get install -y software-properties-common && \
    add-apt-repository 'deb http://mirrors.kernel.org/ubuntu hirsute main universe' && \
    apt-get update && \
    apt-get install -y gcc-11 g++-11 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 70 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9 --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-9 --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-9 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11 --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-11 --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-11;
   
# Install spack
WORKDIR /opt
RUN git clone --depth 1 https://github.com/spack/spack
ENV PATH=/opt/spack/bin:$PATH

# Use the autamus build cache for faster install
# Note that autamus currently uses 18.04
RUN python3 -m pip install botocore boto3 && \
    spack mirror add autamus s3://autamus-cache && \
    curl http://s3.amazonaws.com/autamus-cache/build_cache/_pgp/FFEB24B0A9D81F6D5597F9900B59588C86C41BE7.pub > key.pub && \
    spack gpg trust key.pub

# Find packages already installed on system, e.g. autoconf
RUN spack external find gcc@11.0.1 autoconf bzip2 git tar xz perl && \
    spack config add 'packages:all:target:[x86_64]' && \
    # Install a new CMake (Tim originally wanted 3.17.1 but spack doesn't have it)
    spack install cmake@${CMAKE_VERSION} perl

# Add cmake to a view
RUN spack view --dependencies no symlink --ignore-conflicts /opt/view cmake@${CMAKE_VERSION} perl
ENV PATH=/opt/view/bin:$PATH

RUN spack external find cmake && \
    spack config add 'packages:cmake:buildable:False'

# Add Dyninst source code here (e.g., from PR or master)
WORKDIR /code
COPY . /code

# Add test code to base container so we can build tests here
RUN git clone https://github.com/dyninst/testsuite /opt/testsuite

# Install Dyninst to its own view
WORKDIR /opt/dyninst-env
RUN . /opt/spack/share/spack/setup-env.sh && \
    spack env create -d . && \
    echo "  concretization: together" >> spack.yaml && \
    spack env activate . && \
    
    # This adds metadata for dyninst to spack.yaml
    spack develop --path /code dyninst@${DYNINST_BRANCH} && \

    # ...but we need spack add to add to the install list!
    spack add dyninst@${DYNINST_BRANCH} && \

    # Add our hard coded versions here.
    spack add boost@${BOOST_VERSION} && \
    spack add elfutils@${ELFUTILS_VERSION} && \
    spack add libiberty@${LIBIBERTY_VERSION} && \
    spack add intel-tbb@${INTELTBB_VERSION} && \
    spack install --reuse
    
# Build tests (but don't run)
COPY ./docker/build.sh build.sh
RUN /bin/bash build.sh
