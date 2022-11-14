ARG base
FROM ${base}

ARG build_jobs=1

LABEL maintainer="@hainest"

COPY . /dyninst/src

# Build Dyninst
RUN cd dyninst && \
    mkdir build && \
    cd build && \
    cmake ../src -DCMAKE_INSTALL_PREFIX=/dyninst && \
    cmake --build . --parallel ${build_jobs} && \
    cmake --install . && \
    cd /dyninst && \
    rm -rf build src/.git
