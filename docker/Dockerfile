ARG ubuntu_version=20.04
FROM ubuntu:${ubuntu_version}

# Build the Dockerfile in this directory, context one level up
# docker build -t dyninst -f Dockerfile ../

LABEL maintainer="@hainest,@vsoch"

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Los_Angeles

# We can use build args to populate the specific versions of dependencies (with defaults here)
ARG BOOST_VERSION=1.71.0
ARG ELFUTILS_VERSION=0.186
ARG LIBIBERTY_VERSION=2.33.1
ARG INTELTBB_VERSION=2020.3
ARG PERL_VERSION=5.30.0
ARG CMAKE_VERSION=3.16.3

# Set the branch name for spack to use (should be master even for PR)
ARG DYNINST_BRANCH=master
ENV DYNINST_BRANCH=${DYNINST_BRANCH}

# Args need to be passed into envars to be used in RUN
ENV BOOST_VERSION=${BOOST_VERSION}
ENV ELFUTILS_VERSION=${ELFUTILS_VERSION}
ENV LIBIBERTY_VERSION=${LIBIBERTY_VERSION}
ENV INTELTBB_VERSION=${INTELTBB_VERSION}
ENV PERL_VERSION=${PERL_VERSION}
ENV CMAKE_VERSION=${CMAKE_VERSION}

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
      xsltproc \
      cmake \
      libboost1.71-all-dev \
      libtbb-dev \
      libxml2-dev \
      m4 \
      libncurses-dev

# Install Clingo for Spack
RUN python3 -m pip install --upgrade pip && \
    python3 -m pip install clingo && \
    dpkg-reconfigure tzdata

# Update gcc to 11.1.1 (otherwise we'd have 9.3.0)
RUN apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y gcc-11 g++-11 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 70 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9 --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-9 --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-9 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11 --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-11 --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-11;
   
# Install spack
WORKDIR /opt
RUN git clone --depth 1 https://github.com/spack/spack
ENV PATH=/opt/spack/bin:$PATH

# Install the boto3 package for using AWS services
# spack uses this for its binary caches
RUN python3 -m pip install botocore boto3

# Find packages already installed on system, e.g. autoconf
RUN spack external find --not-buildable gcc autoconf bzip2 git tar xz perl cmake m4 ncurses && \
    spack config add 'packages:all:target:[x86_64]'

# 'spack external find' doesn't work on libraries
RUN printf "\n\
  boost:\n\
    externals:\n\
    - spec: boost@${BOOST_VERSION}\n\
      prefix: /usr\n\
    buildable: false\n\
  intel-tbb:\n\
    externals:\n\
    - spec: intel-tbb@${INTELTBB_VERSION}\n\
      prefix: /usr\n\
    buildable: false\n\
  libxml2:\n\
    externals:\n\
    - spec: libxml2@2.19.10\n\
      prefix: /usr\n\
    buildable: false\n\
" >> ~/.spack/packages.yaml

# Add Dyninst source code here (e.g., from PR or master)
WORKDIR /code
COPY . /code

# Install Dyninst to its own view
WORKDIR /opt/dyninst-env
RUN . /opt/spack/share/spack/setup-env.sh && \
    spack env create -d . && \
    echo "  concretizer:\n    unify: true" >> spack.yaml && \
    spack env activate . && \
    spack add cmake@${CMAKE_VERSION} && \
    spack add perl@${PERL_VERSION} && \
    spack add boost@${BOOST_VERSION} && \
    spack add elfutils@${ELFUTILS_VERSION} && \
    spack add libiberty@${LIBIBERTY_VERSION}+pic && \
    spack add intel-tbb@${INTELTBB_VERSION} && \
    spack install --reuse
    
# Build Dyninst
COPY ./docker/build.sh build.sh
RUN /bin/bash build.sh
