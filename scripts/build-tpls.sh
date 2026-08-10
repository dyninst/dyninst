#!/usr/bin/env bash
#
# Build Dyninst's third-party libraries (oneTBB, elfutils, libiberty) from source.
#
# Dyninst's CMake requires all three to be present and has no download fallback:
# cmake/tpls/Dyninst{TBB,ElfUtils,LibIberty}.cmake each call find_package(REQUIRED).
# Distro packages lag well behind the versions Dyninst is validated against as part
# of rocprofiler-systems, so CI builds them here instead. See scripts/tpl-versions.env.
#
# Usage:
#   build-tpls.sh --prefix DIR [--jobs N] [--skip-prereqs]
#
# Produces one root per library, mirroring the rocprofiler-systems layout so that
# binutils' generic headers (dwarf2.h, demangle.h, ...) cannot shadow elfutils':
#
#   $PREFIX/tbb       pass to cmake as -DTBB_ROOT_DIR
#   $PREFIX/elfutils  pass to cmake as -DElfUtils_ROOT_DIR
#   $PREFIX/binutils  pass to cmake as -DLibIberty_ROOT_DIR
#
# The prefix is self-describing: a stamp file records the versions it was built
# from, so a restored CI cache built from different versions is rebuilt rather
# than silently reused.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tpl-versions.env
source "${script_dir}/tpl-versions.env"

prefix=""
jobs="$(nproc 2>/dev/null || echo 2)"
skip_prereqs=0

usage() {
    sed -n '3,25p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix) prefix="$2"; shift 2 ;;
        --jobs|-j) jobs="$2"; shift 2 ;;
        --skip-prereqs) skip_prereqs=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "error: unknown argument '$1'" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ -z "${prefix}" ]]; then
    echo "error: --prefix is required" >&2
    exit 2
fi

mkdir -p "${prefix}"
prefix="$(cd "${prefix}" && pwd)"

tbb_root="${prefix}/tbb"
elfutils_root="${prefix}/elfutils"
binutils_root="${prefix}/binutils"

stamp="${prefix}/.tpl-versions"
want_stamp="onetbb=${ONETBB_VERSION} elfutils=${ELFUTILS_VERSION} binutils=${BINUTILS_VERSION}"

if [[ -f "${stamp}" ]] && [[ "$(cat "${stamp}")" == "${want_stamp}" ]]; then
    echo "Third-party libraries already present at ${prefix} (${want_stamp}); nothing to do."
    exit 0
fi

echo "Building third-party libraries into ${prefix}"
echo "  ${want_stamp}"
echo "  jobs: ${jobs}"

install_prereqs() {
    if ! command -v apt-get >/dev/null 2>&1; then
        echo "error: only apt-based images are supported today." >&2
        echo "       Re-run with --skip-prereqs after installing the equivalents of:" >&2
        echo "       bzip2 curl git m4 make pkg-config zlib libzstd libbz2 liblzma (all -dev)" >&2
        exit 1
    fi
    apt-get update -qq
    apt-get install -y -qq --no-install-recommends \
        bzip2 ca-certificates curl git m4 make pkg-config \
        zlib1g-dev libzstd-dev libbz2-dev liblzma-dev
}

build_tbb() {
    echo "::group::Build oneTBB ${ONETBB_VERSION}"
    local src="${workdir}/oneTBB"
    git clone --depth 1 --branch "v${ONETBB_VERSION}" \
        https://github.com/uxlfoundation/oneTBB.git "${src}"

    # TBB_TEST=OFF skips the (slow) test tree; TBB_STRICT=OFF keeps oneTBB's own
    # -Werror from failing the build on whichever compiler the image ships.
    cmake -S "${src}" -B "${workdir}/tbb-build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${tbb_root}" \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DTBB_TEST=OFF \
        -DTBB_STRICT=OFF \
        -DTBB_DISABLE_HWLOC_AUTOMATIC_SEARCH=ON
    cmake --build "${workdir}/tbb-build" --parallel "${jobs}" \
        --target tbb tbbmalloc tbbmalloc_proxy
    cmake --install "${workdir}/tbb-build"
    echo "::endgroup::"
}

build_elfutils() {
    echo "::group::Build elfutils ${ELFUTILS_VERSION}"
    local tarball="elfutils-${ELFUTILS_VERSION}.tar.bz2"
    local src="${workdir}/elfutils-${ELFUTILS_VERSION}"

    curl -fsSL --retry 3 --retry-delay 5 -o "${workdir}/${tarball}" \
        "https://sourceware.org/elfutils/ftp/${ELFUTILS_VERSION}/${tarball}" \
        || curl -fsSL --retry 3 --retry-delay 5 -o "${workdir}/${tarball}" \
            "https://mirrors.kernel.org/sourceware/elfutils/${ELFUTILS_VERSION}/${tarball}"
    tar -xf "${workdir}/${tarball}" -C "${workdir}"

    # Flags mirror rocprofiler-systems' DyninstElfUtils.cmake. -fPIC because
    # Dyninst links these into shared libraries. debuginfod is disabled to match
    # Dyninst's ENABLE_DEBUGINFOD default of OFF; enabling one without the other
    # produces a find_package component mismatch.
    (
        cd "${src}"
        CFLAGS="-fPIC -O3 -Wno-error=maybe-uninitialized" \
        CXXFLAGS="-fPIC -O3 -Wno-error=maybe-uninitialized" \
        LDFLAGS="-Wl,-rpath,${elfutils_root}/lib -pthread" \
        ./configure \
            --prefix="${elfutils_root}" \
            --libdir="${elfutils_root}/lib" \
            --enable-install-elfh \
            --enable-thread-safety \
            --disable-libdebuginfod \
            --disable-debuginfod \
            --disable-nls
        make install "-j${jobs}"
    )
    echo "::endgroup::"
}

build_libiberty() {
    echo "::group::Build libiberty from binutils ${BINUTILS_VERSION}"
    local tarball="binutils-${BINUTILS_VERSION}.tar.gz"
    local src="${workdir}/binutils-${BINUTILS_VERSION}"

    curl -fsSL --retry 3 --retry-delay 5 -o "${workdir}/${tarball}" \
        "https://ftpmirror.gnu.org/gnu/binutils/${tarball}" \
        || curl -fsSL --retry 3 --retry-delay 5 -o "${workdir}/${tarball}" \
            "https://mirrors.kernel.org/sourceware/binutils/releases/${tarball}"
    tar -xf "${workdir}/${tarball}" -C "${workdir}"

    mkdir -p "${binutils_root}/lib" "${binutils_root}/include"

    # Build only the libiberty subtree: a full binutils build additionally needs
    # bison/flex/texinfo. MAKEINFO=true no-ops the doc rules that would otherwise
    # require Texinfo.
    (
        cd "${src}"
        CFLAGS="-fPIC -O3 -Wno-error" \
        CXXFLAGS="-fPIC -O3 -Wno-error" \
        MAKEINFO=true \
        ./configure --prefix="${binutils_root}"
        make MAKEINFO=true "-j${jobs}" all-libiberty
    )

    install -C "${src}/libiberty/libiberty.a" "${binutils_root}/lib/"
    install -C -m 644 "${src}"/include/*.h "${binutils_root}/include/"
    echo "::endgroup::"
}

workdir="$(mktemp -d)"
trap 'rm -rf "${workdir}"' EXIT

if [[ "${skip_prereqs}" -eq 0 ]]; then
    install_prereqs
fi

build_tbb
build_elfutils
build_libiberty

echo "${want_stamp}" > "${stamp}"

echo "Third-party libraries installed:"
echo "  TBB_ROOT_DIR       = ${tbb_root}"
echo "  ElfUtils_ROOT_DIR  = ${elfutils_root}"
echo "  LibIberty_ROOT_DIR = ${binutils_root}"
