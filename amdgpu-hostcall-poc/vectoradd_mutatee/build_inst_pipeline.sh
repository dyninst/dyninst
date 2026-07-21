#!/usr/bin/env bash
# build_inst_pipeline.sh <exe> — offline: extract app co, instrument (hc_*), sync note
# to KD, wrap as a fatbin bundle. Emits <exe>.co / .inst.co / .inst.synced.co / .bundle.
set -euo pipefail
EXE="$1"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE="$(cd "$HERE/.." && pwd)"
ROCM=/opt/rocm-7.0.2
OBJDUMP=$ROCM/lib/llvm/bin/llvm-objdump
BUNDLER=$ROCM/lib/llvm/bin/clang-offload-bundler
TARGET='hipv4-amdgcn-amd-amdhsa--gfx908:sramecc+:xnack-'
KERNEL='_Z9vectoraddPfPKfS1_i'
DYNINST_PREFIX=/home/wuxx1279/bin/dynamd
export DYNINSTAPI_RT_LIB="$DYNINST_PREFIX/lib64/libdyninstAPI_RT.so"
export LD_LIBRARY_PATH="$DYNINST_PREFIX/lib64:${LD_LIBRARY_PATH:-}"
MUT="$BASE/examples/test_amdgpu_instrument/test_amdgpu_instrument.bin"
LIB="$BASE/hostcall_lib/hostcall_lib.aliased.elf"

echo ">> [1] extract app co from $EXE"
$OBJDUMP --offloading "$EXE" >/dev/null 2>&1
APP="$(ls -t "$EXE".0.hipv4*gfx908* 2>/dev/null | head -1)"
[ -z "$APP" ] && { echo "extract failed"; exit 1; }
cp -f "$APP" "$EXE.co"

echo ">> [2] instrument (hc_open@entry / hc_write@sites / hc_close@exit)"
"$MUT" "$EXE.co" "$EXE.inst.co" "$KERNEL" "$LIB" >/dev/null 2>&1

echo ">> [3] sync .note to bumped KD"
cp -f "$EXE.inst.co" "$EXE.inst.synced.co"
python3 "$HERE/sync_note_from_kd.py" "$EXE.inst.synced.co" >/dev/null

echo ">> [4] wrap as fatbin bundle"
printf '' > /tmp/empty.host
$BUNDLER --type=o --targets=host-x86_64-unknown-linux-gnu-,$TARGET \
  --input=/tmp/empty.host --input="$EXE.inst.synced.co" --output="$EXE.bundle" 2>/dev/null
echo ">> done: $EXE.{co,inst.co,inst.synced.co,bundle}"
