#!/usr/bin/env python3
"""
check_link.py — statically verify that dyninst-inserted cross-object calls will
resolve at hsa_executable_freeze, without needing a GPU.

For every relocation in the CONSUMER object whose target symbol starts with the
prefix (default .dyninst.), it checks:
  1. the consumer has a matching *undefined* symbol of that name (the reference);
  2. some PROVIDER defines that name as STT_OBJECT in its .symtab — the table the
     AMD GPU loader actually resolves from (it ignores .dynsym).

Usage:
  ./check_link.py consumer.hsaco provider1.hsaco [provider2.hsaco ...]
                  [--prefix .dyninst.]
Exit status is non-zero if any reference is unresolved.
"""
import argparse
import re
import subprocess
import sys

import yaml


def load_elf_yaml(path: str, obj2yaml: str) -> dict:
    text = subprocess.check_output([obj2yaml, path]).decode()
    text = text.replace("--- !ELF", "---", 1)
    # Quote all-digit hex Content/Desc so PyYAML keeps them as strings — but skip
    # values obj2yaml already quoted (it quotes ambiguous all-digit blobs itself).
    def _quote(m):
        val = m.group(2)
        return m.group(0) if val[:1] in ("'", '"') else f"{m.group(1)}'{val}'"
    text = re.sub(r"^(\s*(?:Content|Desc):\s*)(\S+)\s*$", _quote, text,
                  flags=re.MULTILINE)
    return yaml.safe_load(text)


def prefixed_reloc_targets(doc: dict, prefix: str):
    """(symbol, reloc_type, section, offset) for relocs against <prefix>* names."""
    out = []
    for sec in doc.get("Sections", []) or []:
        if sec.get("Type") in ("SHT_RELA", "SHT_REL"):
            for r in sec.get("Relocations", []) or []:
                sym = r.get("Symbol", "")
                if isinstance(sym, str) and sym.startswith(prefix):
                    out.append((sym, r.get("Type"), sec.get("Name"), r.get("Offset")))
    return out


def undefined_names(doc: dict) -> set:
    names = set()
    for key in ("DynamicSymbols", "Symbols"):
        for s in doc.get(key) or []:
            if "Section" not in s and s.get("Name"):  # SHN_UNDEF
                names.add(s["Name"])
    return names


def symtab_objects(doc: dict) -> set:
    """Defined STT_OBJECT names in .symtab (what the loader resolves from)."""
    names = set()
    for s in doc.get("Symbols") or []:
        if s.get("Type") == "STT_OBJECT" and "Section" in s and s.get("Name"):
            names.add(s["Name"])
    return names


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("consumer")
    ap.add_argument("providers", nargs="+")
    ap.add_argument("--prefix", default=".dyninst.")
    ap.add_argument("--obj2yaml", default="obj2yaml")
    args = ap.parse_args()

    consumer = load_elf_yaml(args.consumer, args.obj2yaml)
    provided = {}  # name -> provider path
    for p in args.providers:
        for name in symtab_objects(load_elf_yaml(p, args.obj2yaml)):
            provided.setdefault(name, p)

    relocs = prefixed_reloc_targets(consumer, args.prefix)
    if not relocs:
        print(f"no relocations against '{args.prefix}*' in {args.consumer}")
        return 1

    undef = undefined_names(consumer)
    ok = True
    for sym, rtype, sec, off in relocs:
        has_ref = sym in undef
        prov = provided.get(sym)
        good = has_ref and prov is not None
        ok = ok and good
        mark = "PASS" if good else "FAIL"
        where = f"{sec}@{off:#x}" if isinstance(off, int) else sec
        detail = []
        if not has_ref:
            detail.append("no UND symbol in consumer")
        if prov is None:
            detail.append("no STT_OBJECT definition in any provider .symtab")
        note = "" if good else "  <- " + "; ".join(detail)
        src = f"defined in {prov}" if prov else "UNRESOLVED"
        print(f"[{mark}] {rtype} {where} -> {sym}  ({src}){note}")

    print("\nOK: all cross-object references resolve" if ok
          else "\nFAILED: unresolved references above")
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
