# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This project is a proof-of-concept for **binary instrumentation of HIP device code objects**. The long-term goal is to instrument HIP kernels at the code object level to inject calls into a separately compiled instrumentation library, without recompiling the target application.

The PoC approach:
1. The **mutatee** (`mutatee/`) contains a HIP kernel that uses device-side function pointers, producing relocation records in its code object.
2. The **instrumentation library** (`device_lib/`) provides a device function (`funptr1`) compiled separately as a code object.
3. Using `obj2yaml` / `yaml2obj`, the mutatee's relocation records are manually edited to redirect a function pointer to `funptr1` (defined in `device_lib`).
4. `preload.so` (injected via `LD_PRELOAD` into `vectorAdd-7.0.2/vectoradd_hip.exe` or another host app) loads both code objects directly through the **HSA loader API** (`hsa_executable_load_agent_code_object`) into a single `hsa_executable_t`, then calls `hsa_executable_freeze` to resolve the cross-object relocation against `funptr1` defined in `device_lib`. Kernels are dispatched via HSA to verify the resolution at runtime.

### Why HSA direct-load instead of `hipModuleLoad`?

`hipModuleLoad` cannot load multiple code objects targeting the same GPU architecture into a single module, so it cannot perform cross-code-object linking between them. The HSA executable API can: loading several `hsaco` objects for the same agent into one `hsa_executable_t` and then freezing it resolves inter-object relocations. This is why the preload bypasses HIP's module loader and uses HSA directly.

Target GPU: AMD Instinct MI100 (gfx908, `sramecc+:xnack-`). ROCm/HIP version: 7.0.2 at `/opt/rocm-7.0.2`.

## Build Commands

**Build the preload shared library:**
```sh
make           # produces preload.so
make clean
```
Uses `hipcc` directly — no CMake or other build system.

**Build the device-only library** (`device_lib/`):
```sh
cd device_lib && make       # produces device_only.exe and code objects
```

**Build the vectorAdd mutatee** (`vectorAdd-7.0.2/`):
```sh
cd vectorAdd-7.0.2 && make  # builds and immediately runs vectoradd_hip.exe
```

## Running

Inject `preload.so` into a HIP application using `LD_PRELOAD`:
```sh
LD_PRELOAD=./preload.so ./mutatee
LD_PRELOAD=./preload.so ./vectorAdd-7.0.2/vectoradd_hip.exe
```

The preload constructor (`setup()`) reads two files from the current working directory:
- `funptr1_gfx908.hsaco` — the instrumentation library code object
- `mutatee/code-objects/1-hipv4-amdgcn-amd-amdhsa--gfx908` — the (edited) mutatee code object

Both must be present. They are loaded via `hsa_executable_load_agent_code_object` into a single executable, which is then frozen so the mutatee's relocation against `funptr1` is resolved against the instrumentation library's definition. After freezing, the preload looks up `fn_array`, manually patches entry `[0]` to the resolved `funptr1` address, and dispatches `call_fn` via HSA to exercise the indirect call.

## Architecture

### `preload.cpp` — the interceptor

Intercepts HIP API calls using `dlsym(RTLD_NEXT, ...)` to chain to the real implementation:

- `__hipRegisterFatBinary` — logs the fat binary wrapper address and binary pointer
- `__hipRegisterFunction` — logs module, host/device function pointers, and device name; populates `FuncLookup` / `metaLookup` maps (partially implemented)
- `hipLaunchKernel` — logs grid/block dimensions, calls real launch, then **forces `hipStreamSynchronize`** after every kernel
- `hipMalloc` — transparent passthrough (interception hook is in place for future use)
- `hipMemcpy` — interceptor exists but is **commented out**

Constructor (`__attribute__((constructor)) setup()`): selects the gfx908 HIP device, finds the matching HSA agent, creates an `hsa_executable_t`, loads both the instrumentation (`funptr1_gfx908.hsaco`) and mutatee code objects into it, and calls `hsa_executable_freeze` to resolve the cross-object relocation. It then looks up `fn_array` / `__hip_cuid_*` symbols via the HSA executable API, patches `fn_array[0]` to the resolved `funptr1` address, creates an HSA queue, allocates kernarg memory, and dispatches `call_fn(v, 0)` directly via an `hsa_kernel_dispatch_packet_t` to verify the indirect call.

Destructor (`__attribute__((destructor)) fini()`): destroys the HSA queue and the HSA executable.

Global state: `FuncLookup` maps host function pointers to `config` structs; `metaLookup` maps device names to configs; `mempool` is a vector of `kernel_launch_tracker` pointers (infrastructure for future data collection).

### `mutatee/` — the actual binary instrumentation target

`test.cpp` defines two global device function pointer arrays (`fn_array`) and two kernels:
- `load_fn_ptr` — initializes the function pointer array and overwrites one entry at runtime
- `call_fn` — invokes a function through the pointer array by index

The device function pointer array produces **relocation records** in the gfx908 code object, which is the modification point for instrumentation. The PoC workflow:
1. Extract the gfx908 code object from `a.out` using `clang-offload-bundler`
2. Dump it to YAML with `obj2yaml` → `code-objects/908.yaml`
3. Edit relocation records in the YAML to redirect a pointer to `funptr1` (from `device_lib`)
4. Rebuild with `yaml2obj` → `code-objects/908.s` (binary code object), replacing `mutatee/code-objects/1-hipv4-amdgcn-amd-amdhsa--gfx908`

The edited code object is then loaded at runtime by `preload.so` alongside `funptr1_gfx908.hsaco` via the HSA executable API (see "Why HSA direct-load" above).

`gfx908.backup` is a backup of the original unmodified code object.

### `device_lib/` — instrumentation library (device-only)

`device_only.cpp` defines `funptr1`, a no-op device function stub that acts as the instrumentation call target. The Makefile builds it into `device_only.exe` and extracts code objects for gfx900, gfx908, gfx90a. `funptr1_gfx908.hsaco` in the root is the extracted gfx908 code object.

`funptr.yaml` is an `obj2yaml` dump of the gfx908 code object — useful for inspecting ELF structure and understanding how the symbol is exported. The `code-objects/` directory holds the per-architecture bundles.

### `vectorAdd-7.0.2/` — host driver application

Standard HIP vectorAdd used only as a host-side application to trigger the HIP runtime so that `LD_PRELOAD` injection can work. It is **not** the binary being instrumented.

## Dyninst hostcall instrumentation (newer workflow)

A second, more general instrumentation path uses a patched **Dyninst** (`dyninst/`) to statically rewrite a GPU code object, injecting GPU→CPU **hostcalls** from a separately-compiled device library. This supersedes the manual YAML-editing PoC above.

Components:
- **`hostcall_lib/`** — device hostcall library. `hostcalls.h` is the shared mailbox ABI (device + host). `hostcall_lib.cpp` defines `gpu_fopen/fwrite/fread` plus nullary trace wrappers `hc_open/hc_write/hc_close` (dyninst's AMDGPU `emitCall` can't pass args, so insertion targets must be nullary). `make` runs the full pipeline (`hipcc --genco` → unbundle → `add_object_aliases.py`) producing **`hostcall_lib.aliased.elf`**, which carries `.dyninst.<fn>` STT_OBJECT call aliases and `.dyninst.<fn>.<key>` register-usage symbols (num_vgpr, etc.).
- **`vectoradd_mutatee/`** — a simple runnable HIP vectoradd built with the **exact matching feature set** (`--offload-arch=gfx908:sramecc+:xnack- -mcode-object-version=6` → ABI ver 4, flags `0xE30`). `make` builds the exe and extracts `vectoradd.co` (the code object dyninst rewrites). Run the exe with `ROCR_VISIBLE_DEVICES=1` (device 0 is gfx900). Cross-object linking REQUIRES identical ABI version + feature flags + scratch mode + xnack between mutatee and library.
- **`examples/test_amdgpu_instrument/`** — the dyninst mutator. Inserts `hc_open` at the target kernel's entry, `hc_close` at its exit, and `hc_write` before every 4th instruction (capped). Built as a standalone CMake project against an installed Dyninst (see `examples/README.md`).
- **`add_object_aliases.py`** — post-build tool: injects the `.dyninst.*` OBJECT aliases (loader resolves OBJECT not FUNC) and, with `--asm <file.s>`, the per-function register-usage symbols; also preserves `e_flags` across the obj2yaml/yaml2obj round-trip.

Patched Dyninst pieces (in `dyninst/`, must be built + installed):
- `emit-amdgpu.C` `bumpCallerKdForCallee()` — on each inserted external call, reads the callee's `.dyninst.<callee>.num_vgpr` and raises the caller kernel descriptor's granted VGPR count so the callee's registers fit (SGPR is already maxed by `AmdgpuPointHandler::maximizeSgprAllocationIfKernel`).
- `BPatch_point.C` — enables `BPatch_callBefore` at exit points for AMDGPU (needed for `hc_close` before `s_endpgm`).

Build + run the whole flow:
```sh
cd dyninst/build && make install          # installs patched Dyninst to /home/wuxx1279/bin/dynamd (lib64)
./instrument.sh                           # builds inputs if needed, runs the mutator, verifies
```
`instrument.sh` produces **`vectoradd_mutatee/vectoradd.inst.co`** and verifies the KD VGPR grant was bumped and the `.dyninst.hc_*` relocations are wired. `vectoradd_mutatee/decode_kd.py` decodes a kernel descriptor's granulated VGPR/SGPR/scratch fields.

The host-side server + HSA launcher now exist (`launcher/hostcall_launcher.cpp`). The full pipeline runs end-to-end (instrument → HSA load+freeze → CPU service thread → `fopen('dyninst_trace.txt')` → trace lines written) **except one remaining register-preservation fault** (an address register gets contaminated across the interleaved hostcalls) and a VGPR-grant overshoot.

**See `HOSTCALL_HANDOFF.md` for the complete state, exact build/run commands, the remaining-bug diagnosis, next steps, and the full list of hard-won gotchas (device selection, ABI match, the add_object_aliases.py ELF-corruption fixes, lane election, s[30:31] return ABI, scalar-cache, etc.).**
