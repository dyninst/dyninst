#!/usr/bin/env python3
"""
add_object_aliases.py — for every defined STT_FUNC in a code object, add a
prefixed STT_OBJECT alias (e.g. funptr1 -> .dyninst.funptr1) at the same address.

Why: the ROCm HSA loader only registers/resolves *variables* (STT_OBJECT) and
kernels across code objects — a plain device function (STT_FUNC) is invisible to
both `hsa_executable_get_symbol_by_name` and cross-object relocation resolution.
Exposing an STT_OBJECT alias at the function's address lets the loader bind an
R_AMDGPU_ABS64 against that name to the real address.

We give the alias a *distinct* name (a prefix) rather than reusing the function
name, because yaml2obj rejects two symbols with the same name in one table
("repeated symbol name"). The original STT_FUNC is left untouched, so the
function stays callable and shows up correctly in disassembly / rocgdb.

This script handles the PROVIDER side (the object that *defines* the function).
On the CONSUMER side, dyninst must emit its inserted call's R_AMDGPU_ABS64
against the prefixed name (.dyninst.funptr1) with a matching UND symbol — the
freeze then binds it to this OBJECT alias.

Pipeline: obj2yaml -> add prefixed aliases -> regenerate .hash and .gnu.hash
(both kept internally consistent) -> reorder .dynsym into the order GNU hash
requires -> yaml2obj.

Usage:
    ./add_object_aliases.py in.hsaco out.hsaco [--prefix .dyninst.]
                            [--tables dynsym|symtab|both]

Notes / assumptions:
  * Aliases are STB_GLOBAL and are appended after existing symbols, so the
    ELF "locals before globals" invariant is preserved (yaml2obj fills sh_info).
  * .dynsym is reordered to [locals][global-undef][global-defined sorted by
    gnu-hash bucket] — the layout DT_GNU_HASH requires. Relocations in ELFYAML
    reference symbols by *name*, so yaml2obj recomputes their indices; reorder
    is safe.
  * yaml2obj recomputes file offsets automatically; it preserves the explicit
    section addresses from obj2yaml (the ROCm loader resolves symbols by
    iterating the symbol table, not via sh_addr / DT_ pointers, which is why the
    stock objects already ship with inconsistent hash addresses and still load).
  * Only *defined* STT_FUNC (those with a Section) get an alias; undefined
    references are skipped.
"""
import argparse
import functools
import operator
import re
import struct
import subprocess
import sys

import yaml


# ---- ELF hash functions -----------------------------------------------------
def elf_hash(name: str) -> int:
    """SysV .hash (DT_HASH) hash."""
    h = 0
    for c in name.encode():
        h = ((h << 4) + c) & 0xFFFFFFFF
        g = h & 0xF0000000
        if g:
            h ^= g >> 24
        h &= ~g & 0xFFFFFFFF
    return h


def gnu_hash(name: str) -> int:
    """GNU .gnu.hash (DT_GNU_HASH) djb2 hash."""
    h = 5381
    for c in name.encode():
        h = (h * 33 + c) & 0xFFFFFFFF
    return h


# ---- YAML round-trip that does not corrupt hex Content/Desc -----------------
def load_elf_yaml(text: str) -> dict:
    # obj2yaml emits "--- !ELF"; strip the tag so PyYAML can load it plainly.
    text = text.replace("--- !ELF", "---", 1)
    # All-digit hex blobs (Content/Desc) would be parsed as ints and lose their
    # leading zeros / width. Quote them so they stay strings — but skip values
    # obj2yaml already quoted (it quotes ambiguous all-digit blobs itself),
    # otherwise we'd produce invalid ''..'' double-quoting.
    def _quote(m):
        val = m.group(2)
        return m.group(0) if val[:1] in ("'", '"') else f"{m.group(1)}'{val}'"
    text = re.sub(r"^(\s*(?:Content|Desc):\s*)(\S+)\s*$", _quote, text,
                  flags=re.MULTILINE)
    return yaml.safe_load(text)


def dump_elf_yaml(doc: dict) -> str:
    # NB: PyYAML <5.1 has no sort_keys arg; key order within a mapping is
    # irrelevant to yaml2obj (it reads by key name), and list order — which
    # does matter (Sections, Symbols) — is always preserved.
    body = yaml.safe_dump(doc, default_flow_style=False, width=4096)
    return "--- !ELF\n" + body


# ---- symbol-table edits -----------------------------------------------------
def is_local(sym: dict) -> bool:
    return sym.get("Binding", "STB_LOCAL") == "STB_LOCAL"


def is_defined(sym: dict) -> bool:
    # A defined symbol names a section; undefined references omit Section.
    return "Section" in sym


def object_alias(sym: dict, prefix: str) -> dict:
    alias = {
        "Name": prefix + sym["Name"],
        "Type": "STT_OBJECT",
        "Section": sym["Section"],
        "Binding": "STB_GLOBAL",
    }
    if "Value" in sym:
        alias["Value"] = sym["Value"]
    if "Size" in sym:
        alias["Size"] = sym["Size"]
    return alias


def collect_defined_funcs(doc: dict) -> dict:
    """Map name -> symbol descriptor for every defined STT_FUNC in either table.

    A device function is often LOCAL and present only in .symtab (the compiler
    doesn't export it to .dynsym). We still need a GLOBAL OBJECT alias in BOTH
    tables, so gather from both (first definition wins).
    """
    funcs = {}
    for key in ("DynamicSymbols", "Symbols"):
        for s in doc.get(key) or []:
            if s.get("Type") == "STT_FUNC" and is_defined(s) and s.get("Name"):
                funcs.setdefault(s["Name"], s)
    return funcs


def add_aliases(symbols: list, funcs: dict, prefix: str) -> int:
    """Append a prefixed STT_OBJECT alias (GLOBAL) for each func not yet aliased
    in this table. Returns the number added.

    The alias MUST land in .symtab: the AMD GPU loader resolves symbols from
    .symtab (a known quirk — it ignores .dynsym), which is why the previously
    working object's OBJECT alias lived there. We populate both tables anyway.
    """
    existing = {s.get("Name") for s in symbols}
    added = 0
    for name, src in funcs.items():
        alias_name = prefix + name
        if alias_name in existing:
            continue
        symbols.append(object_alias(src, prefix))
        existing.add(alias_name)
        added += 1
    return added


# ---- per-function register-usage export ------------------------------------
# The AMDGPU backend records each function's post-register-allocation resource
# usage as ".set .L<fn>.<key>, <N>" directives in its assembly (-S output).
# These are the exact numbers LLVM folds into a *kernel's* .kd; for non-kernel
# device functions the ".L" prefix makes them assembler-locals that are stripped
# from the object. We re-export them as durable SHN_ABS symbols
# "<prefix><fn>.<key>" so an instrumentation tool (dyninst) can read a callee's
# footprint and bump the caller kernel descriptor's granted register counts.
REGUSAGE_KEYS = (
    "num_vgpr", "num_agpr", "numbered_sgpr", "private_seg_size",
    "uses_vcc", "uses_flat_scratch",
)

# A '.set NAME, EXPR' directive. NAME may be a '.L' assembler-local; EXPR is a
# plain int for a leaf function, or an expression for a NON-LEAF one.
_SET_RE = re.compile(r"^\s*\.set\s+(?P<name>[.\w$@]+)\s*,\s*(?P<expr>.+?)\s*$")
# Reg-usage '.set' names look like '.L<fn>.<key>'.
_REGUSAGE_NAME_RE = re.compile(
    r"^\.L(?P<fn>.+)\.(?P<key>" + "|".join(REGUSAGE_KEYS) + r")$")
# A symbol reference inside an expression (LLVM uses the '.L' local prefix).
_SYM_REF_RE = re.compile(r"\.L[.\w$@]+")


def _make_regusage_evaluator(defs: dict):
    """Return a memoized evaluator resolving a '.set' symbol to an integer.

    A NON-LEAF function's resource usage is emitted as an EXPRESSION over its
    callees' symbols, e.g. (when the callee is visible in the same compile):
        .set .Lfoo.num_vgpr,         max(1, .Lbar.num_vgpr)
        .set .Lfoo.private_seg_size, 16+(max(.Lbar.private_seg_size))
        .set .Lfoo.uses_flat_scratch, or(0, .Lbar.uses_flat_scratch)
    The value we want is exactly what the compiler already computed; evaluate it
    by substituting each referenced '.L' symbol with its resolved integer and
    evaluating the small max/min/or/and/arith grammar. A reference to a symbol
    that is not defined here (e.g. 'amdgpu.max_num_vgpr', which a STANDALONE
    compile emits for an unresolved external call) raises, so the caller falls
    back to the whole-object-max heuristic rather than exporting a bogus value.
    """
    memo = {}
    # LLVM uses max(x)/max(a,b,...) with one OR many args, so accept varargs
    # (Python's builtin max() rejects a single scalar).
    env = {
        "max": lambda *a: max(a), "min": lambda *a: min(a),
        "or": lambda *a: functools.reduce(operator.or_, (int(x) for x in a), 0),
        "and": lambda *a: functools.reduce(operator.and_, (int(x) for x in a), ~0),
    }

    def ev(name: str) -> int:
        if name in memo:
            if memo[name] is None:
                raise ValueError(f"cyclic .set reference: {name}")
            return memo[name]
        if name not in defs:
            raise KeyError(name)
        memo[name] = None  # cycle guard
        expr = defs[name]
        # Substitute referenced symbols with their resolved integer values.
        expr = _SYM_REF_RE.sub(lambda m: f"({ev(m.group(0))})", expr)
        # 'or'/'and' are Python keywords; rewrite the call forms to helper names.
        expr = re.sub(r"\bor\s*\(", "or_(", expr)
        expr = re.sub(r"\band\s*\(", "and_(", expr)
        local = dict(env, or_=env["or"], and_=env["and"])
        val = int(eval(expr, {"__builtins__": {}}, local))  # trusted compiler asm
        memo[name] = val
        return val

    return ev


def parse_asm_regusage(path: str) -> dict:
    """Parse per-function resource usage from compiler assembly (-S output).

    Returns { func_name: { key: int } }. func_name matches the object's STT_FUNC
    symbol name (e.g. 'gpu_fopen' for extern "C", or the mangled name otherwise).

    Leaf functions emit '.set .L<fn>.<key>, <N>'; non-leaf functions emit an
    expression over their callees' '.L' symbols. Both are handled: all '.set'
    directives are collected first, then each reg-usage symbol is evaluated. A
    symbol whose expression references something undefined in this file (worst-case
    marker from a standalone compile) is skipped, leaving the emitter's
    whole-object-max fallback to cover it.
    """
    defs = {}
    with open(path) as fh:
        for line in fh:
            m = _SET_RE.match(line)
            if m:
                defs[m.group("name")] = m.group("expr")
    ev = _make_regusage_evaluator(defs)
    out = {}
    for name in defs:
        rm = _REGUSAGE_NAME_RE.match(name)
        if not rm:
            continue
        try:
            out.setdefault(rm.group("fn"), {})[rm.group("key")] = ev(name)
        except Exception as e:  # unresolved/cyclic/parse -> fall back to object-max
            sys.stderr.write(f"  note: unresolved reg-usage {name}: {e}\n")
    return out


def add_regusage_symbols(symbols: list, funcs: dict, regusage: dict,
                         prefix: str) -> int:
    """Append GLOBAL SHN_ABS symbols '<prefix><fn>.<key>' = value for each aliased
    function that has regalloc data. Returns the number added.

    .symtab only: a tool reads register usage from the symbol table, and the
    values are never relocation targets, so there's no need to touch .dynsym /
    the GNU-hash layout. GLOBAL + appended keeps 'locals before globals' intact.
    """
    existing = {s.get("Name") for s in symbols}
    added = 0
    for name in funcs:
        ru = regusage.get(name)
        if not ru:
            continue
        for key in REGUSAGE_KEYS:
            if key not in ru:
                continue
            sym_name = f"{prefix}{name}.{key}"
            if sym_name in existing:
                continue
            symbols.append({
                "Name": sym_name,
                "Index": "SHN_ABS",
                "Binding": "STB_GLOBAL",
                "Value": ru[key],
            })
            existing.add(sym_name)
            added += 1
    return added


# ---- e_flags preservation ---------------------------------------------------
def _eflags_offset(path: str) -> int:
    """Byte offset of e_flags in the ELF header (0x30 for ELF64, 0x24 for ELF32)."""
    with open(path, "rb") as fh:
        ei_class = fh.read(5)[4]  # EI_CLASS: 1=ELF32, 2=ELF64
    return 0x30 if ei_class == 2 else 0x24


def preserve_eflags(src: str, dst: str) -> None:
    """Copy e_flags verbatim from src ELF header to dst.

    The obj2yaml/yaml2obj round-trip (system LLVM) silently drops the AMDGPU
    sramecc feature bit (0xE30 -> 0x230), which would make the object's target-id
    mismatch a co-loaded mutatee. Restore the original 4-byte e_flags directly.
    """
    off = _eflags_offset(src)
    with open(src, "rb") as fh:
        fh.seek(off)
        flags = fh.read(4)
    with open(dst, "r+b") as fh:
        fh.seek(off)
        fh.write(flags)


def fix_load_segment(path: str) -> None:
    """Repair the first PT_LOAD so the read-only sections (.rodata etc.) load.

    The obj2yaml/yaml2obj round-trip (LLVM 15) rebuilds the first PT_LOAD with an
    inconsistent file-offset<->vaddr delta (e.g. offset=0x238, vaddr=0), while it
    lays the read-only allocable sections at sh_offset == sh_addr (delta 0). So
    those sections (notably .rodata) fall OUTSIDE the segment and are never loaded
    -> GPU reads of string/const data return zeros. yaml2obj's delta-0 section
    placement means a congruent segment [0, end-of-RO-group) covers them. Set the
    first PT_LOAD to offset=vaddr=0 and span the read-only allocable group.
    ELF64 little-endian only (all our gfx908 code objects).
    """
    d = bytearray(open(path, "rb").read())
    if len(d) < 0x40 or d[:4] != b"\x7fELF" or d[4] != 2:
        return  # not ELF64
    u16 = lambda o: struct.unpack_from("<H", d, o)[0]
    u64 = lambda o: struct.unpack_from("<Q", d, o)[0]
    e_phoff, e_shoff = u64(0x20), u64(0x28)
    e_phentsize, e_phnum = u16(0x36), u16(0x38)
    e_shentsize, e_shnum = u16(0x3a), u16(0x3c)
    SHF_WRITE, SHF_ALLOC, SHF_EXEC = 0x1, 0x2, 0x4
    ro_end = 0
    for i in range(e_shnum):
        b = e_shoff + i * e_shentsize
        fl, ad, sz = u64(b + 8), u64(b + 16), u64(b + 32)
        if (fl & SHF_ALLOC) and not (fl & (SHF_WRITE | SHF_EXEC)):
            ro_end = max(ro_end, ad + sz)
    if ro_end == 0:
        return
    for i in range(e_phnum):
        b = e_phoff + i * e_phentsize
        if struct.unpack_from("<I", d, b)[0] == 1:  # first PT_LOAD
            struct.pack_into("<Q", d, b + 8, 0)        # p_offset
            struct.pack_into("<Q", d, b + 16, 0)       # p_vaddr
            struct.pack_into("<Q", d, b + 24, 0)       # p_paddr
            struct.pack_into("<Q", d, b + 32, ro_end)  # p_filesz
            struct.pack_into("<Q", d, b + 40, ro_end)  # p_memsz
            break
    open(path, "wb").write(d)


# ---- hash-table regeneration ------------------------------------------------
def regen_gnu_hash(sections: list, dynsyms: list) -> list:
    """Reorder dynsyms for GNU hash and rewrite the SHT_GNU_HASH section.

    Returns the reordered dynsym list. Full .dynsym indices are offset by 1
    because yaml2obj prepends the implicit null symbol at index 0.
    """
    gnu = next((s for s in sections if s.get("Type") == "SHT_GNU_HASH"), None)

    locals_ = [s for s in dynsyms if is_local(s)]
    g_undef = [s for s in dynsyms if not is_local(s) and not is_defined(s)]
    g_def = [s for s in dynsyms if not is_local(s) and is_defined(s)]

    nbuckets = max(1, len(g_def))
    # Sort the hashed (defined, global) symbols by bucket — required so each
    # bucket's chain is contiguous and buckets are non-decreasing by index.
    g_def.sort(key=lambda s: gnu_hash(s["Name"]) % nbuckets)
    ordered = locals_ + g_undef + g_def

    if gnu is None:
        return ordered

    symndx = 1 + len(locals_) + len(g_undef)  # +1 for implicit null at index 0
    shift2 = 6
    maskwords = 1  # one 64-bit bloom word (power of two, valid for small tables)
    bloom = [0] * maskwords
    buckets = [0] * nbuckets
    values = []

    hashes = [gnu_hash(s["Name"]) for s in g_def]
    for i, h in enumerate(hashes):
        bloom[(h // 64) % maskwords] |= (1 << (h % 64)) | (1 << ((h >> shift2) % 64))
    for i, h in enumerate(hashes):
        b = h % nbuckets
        if buckets[b] == 0:
            buckets[b] = symndx + i
    for i, h in enumerate(hashes):
        last = (i == len(hashes) - 1) or (hashes[i + 1] % nbuckets != h % nbuckets)
        values.append((h & ~1) | (1 if last else 0))

    gnu.pop("Content", None)
    gnu["Header"] = {"SymNdx": symndx, "Shift2": shift2}
    gnu["BloomFilter"] = bloom
    gnu["HashBuckets"] = buckets
    gnu["HashValues"] = values
    return ordered


def regen_sysv_hash(sections: list, dynsyms: list) -> None:
    """Rewrite the SHT_HASH section over the final dynsym order."""
    h = next((s for s in sections if s.get("Type") == "SHT_HASH"), None)
    if h is None:
        return
    full = [None] + dynsyms  # index 0 is the implicit null symbol
    nsym = len(full)
    nbucket = max(1, nsym)
    bucket = [0] * nbucket
    chain = [0] * nsym
    for i in range(1, nsym):
        name = full[i].get("Name", "")
        if not name:
            continue
        b = elf_hash(name) % nbucket
        chain[i] = bucket[b]
        bucket[b] = i
    h.pop("Content", None)
    h["Bucket"] = bucket
    h["Chain"] = chain


# ---- preserve original allocable-section file offsets -----------------------
def read_alloc_section_offsets(path: str) -> dict:
    """Map {section_name: sh_offset} for every SHF_ALLOC section in the input ELF.

    obj2yaml records only Address/AddressAlign, so yaml2obj recomputes file offsets
    from scratch — and packs them more loosely than the original linker (which uses a
    tighter, sometimes overlapping, layout). That drifts loadable sections' file offsets
    away from their addresses (sh_offset != sh_addr), and since the RO PT_LOAD maps
    file->vaddr 1:1 (delta 0; see fix_load_segment), a byte the GPU reads at vaddr V then
    comes from the wrong file offset — e.g. .rodata string constants (whose vaddr is
    baked into the code as an s_getpc-relative offset) read back as zeros. Pinning
    ShOffset = original offset for allocable sections makes yaml2obj reproduce the
    original delta-0 loadable layout. ELF64 little-endian.
    """
    d = open(path, "rb").read()
    if len(d) < 0x40 or d[:4] != b"\x7fELF" or d[4] != 2:
        return {}
    u16 = lambda o: struct.unpack_from("<H", d, o)[0]
    u32 = lambda o: struct.unpack_from("<I", d, o)[0]
    u64 = lambda o: struct.unpack_from("<Q", d, o)[0]
    e_shoff, e_shentsize = u64(0x28), u16(0x3a)
    e_shnum, e_shstrndx = u16(0x3c), u16(0x3e)
    sh = lambda i: e_shoff + i * e_shentsize
    strtab = u64(sh(e_shstrndx) + 24)
    SHF_ALLOC = 0x2
    out = {}
    for i in range(e_shnum):
        b = sh(i)
        flags, off = u64(b + 8), u64(b + 24)
        if flags & SHF_ALLOC:
            no = u32(b + 0)
            end = d.index(b"\x00", strtab + no)
            out[d[strtab + no:end].decode("latin-1")] = off
    return out


# ---- driver -----------------------------------------------------------------
def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--prefix", default=".dyninst.",
                    help="prefix for the STT_OBJECT alias name (default: .dyninst.)")
    ap.add_argument("--tables", choices=["dynsym", "symtab", "both"], default="both",
                    help="which symbol table(s) to add aliases to (default: both)")
    ap.add_argument("--asm", default=None,
                    help="compiler assembly (-S output) to import per-function "
                         "VGPR/SGPR/scratch usage as '<prefix><fn>.<key>' ABS symbols")
    ap.add_argument("--obj2yaml", default="obj2yaml")
    ap.add_argument("--yaml2obj", default="yaml2obj")
    args = ap.parse_args()

    text = subprocess.check_output([args.obj2yaml, args.input]).decode()
    doc = load_elf_yaml(text)
    sections = doc.get("Sections", [])

    funcs = collect_defined_funcs(doc)
    added = 0
    dynsym_added = 0
    if args.tables in ("dynsym", "both") and doc.get("DynamicSymbols") is not None:
        dynsym_added = add_aliases(doc["DynamicSymbols"], funcs, args.prefix)
        added += dynsym_added
    if args.tables in ("symtab", "both") and doc.get("Symbols") is not None:
        added += add_aliases(doc["Symbols"], funcs, args.prefix)

    reg_added = 0
    if args.asm:
        regusage = parse_asm_regusage(args.asm)
        if doc.get("Symbols") is None:
            doc["Symbols"] = []
        reg_added = add_regusage_symbols(doc["Symbols"], funcs, regusage, args.prefix)

    # Regenerate the dynamic hash tables ONLY when we actually changed .dynsym. When we
    # add symbols to .symtab only (the common path), .dynsym and its hashes are untouched
    # and already valid — regenerating them would rewrite .gnu.hash to a different size,
    # and since the original layout has .gnu.hash/.hash at overlapping offsets (which we
    # preserve via ShOffset below), a size change corrupts the overlap -> invalid object.
    if dynsym_added and doc.get("DynamicSymbols"):
        ordered = regen_gnu_hash(sections, doc["DynamicSymbols"])
        doc["DynamicSymbols"] = ordered
        regen_sysv_hash(sections, ordered)

    # Pin allocable sections to their original file offsets so yaml2obj reproduces the
    # original delta-0 loadable layout (otherwise .rodata drifts off its baked vaddr and
    # GPU string reads return zeros). Non-allocable tables (.symtab/.strtab) float — they
    # grow with the added symbols and sit after the loadable image.
    alloc_off = read_alloc_section_offsets(args.input)
    for s in sections:
        nm = s.get("Name")
        if nm in alloc_off and "ShOffset" not in s:
            s["ShOffset"] = alloc_off[nm]

    out_yaml = dump_elf_yaml(doc)
    proc = subprocess.run([args.yaml2obj, "-o", args.output],
                          input=out_yaml.encode())
    if proc.returncode != 0:
        sys.stderr.write("yaml2obj failed\n")
        return 1

    # The yaml round-trip drops the AMDGPU sramecc e_flag; restore it verbatim.
    preserve_eflags(args.input, args.output)
    fix_load_segment(args.output)

    print(f"added {added} STT_OBJECT alias(es) with prefix '{args.prefix}'"
          + (f" and {reg_added} register-usage symbol(s)" if reg_added else "")
          + f"; wrote {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
