#!/usr/bin/env python3
"""
Patch the yaml2obj-generated mutatee gfx908 code object to fix VAddr/FileOffset
mismatches introduced when 'funptr1' was added to DynamicSymbols (growing .dynsym
by 24 bytes and .dynstr by 8 bytes, shifting all subsequent sections).

Assumes the PT_LOAD has already been binary-patched (p_offset=0x0, p_filesz=0xc2a).

Run from the repo root or adjust FNAME as needed.
"""
import struct

FNAME = "mutatee/code-objects/1-hipv4-amdgcn-amd-amdhsa--gfx908"

with open(FNAME, "r+b") as f:
    data = bytearray(f.read())

def get_u64(off): return struct.unpack_from("<Q", data, off)[0]
def get_u32(off): return struct.unpack_from("<I", data, off)[0]
def set_u64(off, v): struct.pack_into("<Q", data, off, v); print(f"  @{off:#x}: set to {v:#x}")
def set_u32(off, v): struct.pack_into("<I", data, off, v); print(f"  @{off:#x}: set to {v:#x}")

# ELF header
e_shoff = get_u64(40)
print(f"Section headers at {e_shoff:#x}")

SZ = 64  # Elf64_Shdr size

def sh_addr_off(ndx): return e_shoff + ndx * SZ + 16

# --- Fix section sh_addr to match sh_offset ---
# .dynsym grew by 0x18 (one extra Elf64_Sym for funptr1 UND)
# .dynstr grew by 0x08 ("funptr1\0")
# Each section below gets its sh_addr corrected to the actual file offset.
print("\n[Section headers]")
for ndx, name, old_addr, new_addr in [
    (3, ".gnu.hash", 0xa38, 0xa50),
    (4, ".hash",     0xa68, 0xa80),
    (5, ".dynstr",   0xaa0, 0xab8),
    (6, ".rela.dyn", 0xaf8, 0xb18),
    (7, ".rodata",   0xb40, 0xb80),
]:
    cur = get_u64(sh_addr_off(ndx))
    assert cur == old_addr, f"section {ndx} ({name}): expected {old_addr:#x} got {cur:#x}"
    set_u64(sh_addr_off(ndx), new_addr)

# --- Fix .dynamic DT_ entries ---
# .dynamic is section [9]
dyn_off = get_u64(e_shoff + 9 * SZ + 24)
dyn_sz  = get_u64(e_shoff + 9 * SZ + 32)
print(f"\n[.dynamic at {dyn_off:#x}, size {dyn_sz:#x}]")

DT_RELA     = 7
DT_STRTAB   = 5
DT_STRSZ    = 10
DT_GNU_HASH = 0x6ffffef5
DT_HASH     = 4
DT_NULL     = 0

patches = {
    DT_RELA:     (0xaf8, 0xb18),   # .rela.dyn new VAddr
    DT_STRTAB:   (0xaa0, 0xab8),   # .dynstr new VAddr
    DT_STRSZ:    (0x52,  0x5a),    # .dynstr grew by 8 for "funptr1\0"
    DT_GNU_HASH: (0xa38, 0xa50),   # .gnu.hash new VAddr
    DT_HASH:     (0xa68, 0xa80),   # .hash new VAddr
}

off = dyn_off
while True:
    tag = get_u64(off)
    val = get_u64(off + 8)
    if tag == DT_NULL:
        break
    if tag in patches:
        old, new = patches[tag]
        assert val == old, f"DT tag {tag:#x}: expected {old:#x}, got {val:#x}"
        set_u64(off + 8, new)
    off += 16

# --- Fix .dynsym kernel descriptor VAddrs ---
# .dynsym is section [2]; each Elf64_Sym is 24 bytes, st_value at offset 8.
# The kernel descriptor symbols sit in .rodata, which moved from 0xb40 to 0xb80.
dsym_off = get_u64(e_shoff + 2 * SZ + 24)
print(f"\n[.dynsym at {dsym_off:#x}]")
ESYM = 24

for ndx, old_val, new_val, label in [
    (3, 0xb40, 0xb80, "_Z11load_fn_ptrPf.kd"),
    (5, 0xb80, 0xbc0, "_Z7call_fnPKfi.kd"),
]:
    sym_base = dsym_off + ndx * ESYM
    cur = get_u64(sym_base + 8)
    assert cur == old_val, f"dynsym[{ndx}] ({label}) st_value: expected {old_val:#x}, got {cur:#x}"
    set_u64(sym_base + 8, new_val)

with open(FNAME, "wb") as f:
    f.write(data)
print("\nPatched OK")
