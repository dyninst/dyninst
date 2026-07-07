#!/usr/bin/env bash
#
# instrument.sh — end-to-end dyninst instrumentation of the vectoradd mutatee.
#
# Injects GPU->CPU hostcalls into the vectoradd kernel using the modified
# example mutator (examples/test_amdgpu_instrument):
#   hc_open  at kernel entry, hc_close at kernel exit,
#   hc_write before every 4th instruction (capped).
# Each inserted call resolves to the separately-compiled device hostcall library
# (hostcall_lib.aliased.elf) and triggers the caller kernel-descriptor VGPR bump.
#
# Requires: our dyninst installed at $DYNINST_PREFIX (cd dyninst/build && make install).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ---- configuration ----------------------------------------------------------
DYNINST_PREFIX="${DYNINST_PREFIX:-/home/wuxx1279/bin/dynamd}"
ROCM="${ROCM:-/opt/rocm-7.0.2}"
KERNEL="${KERNEL:-_Z9vectoraddPfPKfS1_i}"          # target kernel (open/close around)
MUTATOR="${MUTATOR:-$HERE/examples/test_amdgpu_instrument/test_amdgpu_instrument.bin}"

MUTATEE="$HERE/vectoradd_mutatee/vectoradd.co"
INSTLIB="$HERE/hostcall_lib/hostcall_lib.aliased.elf"
OUT="$HERE/vectoradd_mutatee/vectoradd.inst.co"
RE="$ROCM/lib/llvm/bin/llvm-readelf"

# ---- (re)build inputs if missing --------------------------------------------
[ -f "$MUTATEE" ] || make -C "$HERE/vectoradd_mutatee" vectoradd.co
[ -f "$INSTLIB" ] || make -C "$HERE/hostcall_lib"
if [ ! -x "$MUTATOR" ]; then
  echo "ERROR: mutator binary not found at $MUTATOR" >&2
  echo "  Build it: cd examples && cmake . -B <bld> -DDyninst_DIR=$DYNINST_PREFIX/lib64/cmake/Dyninst" >&2
  echo "            cmake --build <bld> --target test_amdgpu_instrument   (then copy it here)" >&2
  exit 1
fi

# ---- run the mutator --------------------------------------------------------
export DYNINSTAPI_RT_LIB="$DYNINST_PREFIX/lib64/libdyninstAPI_RT.so"
export LD_LIBRARY_PATH="$DYNINST_PREFIX/lib64:${LD_LIBRARY_PATH:-}"

echo ">> instrumenting $MUTATEE -> $OUT"
rm -f "$OUT"
"$MUTATOR" "$MUTATEE" "$OUT" "$KERNEL" "$INSTLIB"

# ---- verify -----------------------------------------------------------------
echo ">> verifying kernel-descriptor VGPR bump"
python3 "$HERE/vectoradd_mutatee/decode_kd.py" "${KERNEL}.kd" "$MUTATEE" "$OUT"
echo ">> hostcall relocations wired into the output:"
"$RE" -r "$OUT" 2>/dev/null | grep -E "\.dyninst\.hc_(open|write|close)" || \
  echo "  (none found — check the run output above)"
echo ">> done: $OUT"
