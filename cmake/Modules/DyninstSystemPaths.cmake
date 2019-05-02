set(DYNINST_SYSTEM_INCLUDE_PATHS
    /usr/include
    /usr/include/x86_64-linux-gnu
    /usr/local/include
    /opt/include
    /opt/local/include
    /sw/include
    ENV CPATH
    ENV PATH)

set(DYNINST_SYSTEM_LIBRARY_PATHS
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
    /usr/lib/x86_64-linux-gnu
    /opt/local/lib
    /opt/local/lib64
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH
    ENV PATH)
