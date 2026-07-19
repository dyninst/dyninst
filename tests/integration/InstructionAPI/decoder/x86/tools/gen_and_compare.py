#!/usr/bin/env python3
"""Differential x86-64 decoder sweep: Dyninst vs GNU objdump.

Generates ~45k encodings covering every one-byte and two-byte opcode
(x none/66/F3/F2 prefixes x ModRM.reg 0-7 x register/memory forms),
every 0F 38 / 0F 3A opcode (x prefixes), and every VEX3 opcode
(maps 1-3 x pp 0-3 x L x W), padded into fixed-size slots of one blob.
Both decoders decode every slot; disagreements are bucketed as:

  mnemonic-mismatch     same encoding, different instruction identity
  length-mismatch       same identity, different instruction length
                        (decode desync -- the most dangerous bucket)
  dyninst-rejects       objdump decodes it, Dyninst does not
  objdump-rejects       Dyninst decodes it, objdump does not
  objdump-splits-prefix objdump treats a prefix as standalone (usually
                        a reserved prefixed encoding; triage manually)

IMPORTANT: neither decoder is ground truth. Verify every actionable
disagreement against the Intel SDM / AMD APM before changing tables.
Known-benign naming differences (AT&T suffixes, cc-code aliases,
imm-specialized pseudo-mnemonics) are filtered by the ALIAS table and
suffix stripping below; extend those when a new benign pattern shows up.

Usage:
  1. Build the slot decoder next to this script (see sweep.cpp header).
  2. python3 gen_and_compare.py
  3. Read the per-bucket summary on stdout and the raw bucket-*.txt
     files written next to this script.

Requires: python3, objdump (binutils) on PATH, ./sweep built from
sweep.cpp in the same directory."""
import subprocess, sys, os, re, collections

SLOT = 24
NOP = 0x90
SP = os.path.dirname(os.path.abspath(__file__))
BLOB = os.path.join(SP, "sweep.bin")

slots = []  # (label, bytes)

def add(label, bs):
    slots.append((label, bytes(bs)))

IMM = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]  # generous immediate/disp fodder

def gen():
    # one-byte opcodes, reg and mem modrm, no prefix
    for op in range(0x100):
        if op == 0x0F:
            continue
        # skip prefix bytes as leading opcode (they'd just shift the window)
        if op in (0x26,0x2E,0x36,0x3E,0x64,0x65,0x66,0x67,0xF0,0xF2,0xF3) or 0x40 <= op <= 0x4F:
            continue
        if op in (0xC4, 0xC5, 0x62, 0x8F):  # VEX/EVEX/XOP escapes with reg modrm are covered separately
            add("1b-%02x-mem" % op, [op, 0x08] + IMM)
            continue
        add("1b-%02x-reg" % op, [op, 0xC8] + IMM)
        add("1b-%02x-mem" % op, [op, 0x08] + IMM)
    # two-byte 0F xx with all mandatory prefixes, reg field 0..7, mod reg/mem
    for op in range(0x100):
        if op == 0x0F:
            continue
        for pfx, pname in ((b"", "np"), (b"\x66", "66"), (b"\xf3", "f3"), (b"\xf2", "f2")):
            for reg in range(8):
                add("2b-%s-%02x-r%d-reg" % (pname, op, reg), list(pfx) + [0x0F, op, 0xC0 | (reg << 3)] + IMM)
                add("2b-%s-%02x-r%d-mem" % (pname, op, reg), list(pfx) + [0x0F, op, 0x00 | (reg << 3)] + IMM)
    # 0F 38 xx / 0F 3A xx
    for esc in (0x38, 0x3A):
        for op in range(0x100):
            for pfx, pname in ((b"", "np"), (b"\x66", "66"), (b"\xf3", "f3"), (b"\xf2", "f2")):
                add("3b%02x-%s-%02x-reg" % (esc, pname, op), list(pfx) + [0x0F, esc, op, 0xC8] + IMM)
                add("3b%02x-%s-%02x-mem" % (esc, pname, op), list(pfx) + [0x0F, esc, op, 0x08] + IMM)
    # VEX3: maps 1-3, pp 0-3, L 0/1, W 0/1, vvvv=0b1111 (unused=0)
    for m in (1, 2, 3):
        for op in range(0x100):
            for pp in range(4):
                for L in (0, 1):
                    for W in (0, 1):
                        b1 = 0xE0 | m           # R=1,X=1,B=1 (inverted -> no ext), mmmmm=m
                        b2 = (W << 7) | (0xF << 3) | (L << 2) | pp  # vvvv=1111 (=unused)
                        add("vex-m%d-pp%d-L%d-W%d-%02x-reg" % (m, pp, L, W, op),
                            [0xC4, b1, b2, op, 0xC9] + IMM)
                        add("vex-m%d-pp%d-L%d-W%d-%02x-mem" % (m, pp, L, W, op),
                            [0xC4, b1, b2, op, 0x09] + IMM)

gen()

with open(BLOB, "wb") as f:
    for label, bs in slots:
        assert len(bs) <= SLOT, label
        f.write(bs + bytes([NOP] * (SLOT - len(bs))))

# --- objdump pass ---
od = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386:x86-64",
                     "--insn-width=16", BLOB],
                    stdout=subprocess.PIPE, universal_newlines=True).stdout
od_at = {}
line_re = re.compile(r"^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*\t?(.*)$")
for line in od.splitlines():
    mm = line_re.match(line)
    if not mm:
        continue
    off = int(mm.group(1), 16)
    nbytes = len(mm.group(2).split())
    text = mm.group(3).strip()
    od_at[off] = (nbytes, text)

# --- dyninst pass ---
dyn = subprocess.run([os.path.join(SP, "sweep"), BLOB, str(SLOT)],
                     stdout=subprocess.PIPE, universal_newlines=True).stdout
dyn_at = {}
for line in dyn.splitlines():
    parts = line.split("|", 4)
    off = int(parts[0], 16)
    dyn_at[off] = (parts[1] == "1", int(parts[2]), parts[3], parts[4] if len(parts) > 4 else "")

# --- known naming aliases (dyninst name -> acceptable objdump names) ---
ALIAS = {
  "jz": {"je"}, "jnz": {"jne"}, "jnbe": {"ja"}, "jnb": {"jae"}, "jb": {"jb"},
  "jbe": {"jbe"}, "jl": {"jl"}, "jle": {"jle"}, "jnl": {"jge"}, "jnle": {"jg"},
  "jns": {"jns"}, "js": {"js"}, "jp": {"jp"}, "jnp": {"jnp"}, "jo": {"jo"}, "jno": {"jno"},
  "setz": {"sete"}, "setnz": {"setne"}, "setnbe": {"seta"}, "setnb": {"setae"},
  "setnle": {"setg"}, "setnl": {"setge"},
  "cmovz": {"cmove"}, "cmovnz": {"cmovne"}, "cmovnbe": {"cmova"}, "cmovnb": {"cmovae"},
  "cmovnle": {"cmovg"}, "cmovnl": {"cmovge"},
  "shl": {"sal", "shl"},
  "cbw": {"cbtw", "cwtl", "cltq"}, "cwde": {"cwtl", "cltq"}, "cdq": {"cltd", "cqto", "cwtd"},
  "ret": {"ret", "retq"}, "ret_near": {"ret", "retq"}, "ret_far": {"lret", "lretq"},
  "int3": {"int3"}, "int": {"int"},
  "movsd": {"movsd"},  # string vs sse distinguished by operands; mnemonic same
  "pushf": {"pushf", "pushfq"}, "popf": {"popf", "popfq"},
  "movabs": {"movabs"}, "mov": {"mov", "movabs"},
  "loopn": {"loopne"}, "loope": {"loope"},
  "fxch": {"fxch"},
}

def norm_dyn(m):
    return m.strip().lower()

def norm_od(text):
    t = text.split()
    if not t:
        return "", ""
    return t[0].lower(), text

# objdump size-suffix stripping: movl->mov etc. Only strip one of b/w/l/q if
# the rest matches the dyninst mnemonic.
def matches(dm, om):
    if dm == om:
        return True
    if om in ALIAS.get(dm, ()):  # alias table
        return True
    if len(om) > 1 and om[:-1] == dm and om[-1] in "bwlq":
        return True
    # objdump adds 'q'/'l' to some like cmpxchg8b? no. iretq vs iret:
    if om.rstrip("q") == dm or om.rstrip("l") == dm or om.rstrip("w") == dm:
        return True
    return False

PREFIX_ONLY = {"lock", "data16", "repz", "repnz", "rep", "addr32", "cs", "ds", "es",
               "fs", "gs", "ss", "rex", "rex.w", "rex.b", "(bad)", "bnd", "notrack", "xacquire", "xrelease"}

buckets = collections.defaultdict(list)
for i, (label, bs) in enumerate(slots):
    off = i * SLOT
    dvalid, dlen, dmnem, dfmt = dyn_at.get(off, (False, 0, "", ""))
    if off not in od_at:
        buckets["no-objdump-line"].append((label, dmnem))
        continue
    olen, otext = od_at[off]
    omnem, _ = norm_od(otext)
    obad = otext.startswith("(bad)") or omnem == "(bad)"
    # objdump printing a bare prefix means it didn't accept prefix+opcode as one insn
    osplit = omnem in PREFIX_ONLY and not obad

    if not dvalid and (obad or osplit):
        continue  # both reject (or objdump waffles) - fine
    if not dvalid and not obad:
        buckets["dyninst-rejects"].append((label, otext))
        continue
    if dvalid and obad:
        buckets["objdump-rejects"].append((label, dmnem, dfmt))
        continue
    if dvalid and osplit:
        buckets["objdump-splits-prefix"].append((label, dmnem, otext))
        continue
    dm = norm_dyn(dmnem)
    if not matches(dm, omnem):
        buckets["mnemonic-mismatch"].append((label, dm, otext))
        continue
    if dlen != olen:
        buckets["length-mismatch"].append((label, dm, dlen, olen, otext))

def collapse(label):
    """Merge the per-slot detail (reg field, VEX L/W) so recurring
    patterns group into one line."""
    s = re.sub(r"-r\d-", "-r*-", label)
    s = re.sub(r"(vex-m\d-pp\d)-L\d-W\d", r"\1-L*-W*", s)
    return s

print("total slots:", len(slots))
for k in sorted(buckets):
    print("\n=== %s: %d ===" % (k, len(buckets[k])))
    sig = collections.Counter()
    example = {}
    for item in buckets[k]:
        key = (collapse(item[0]), tuple(str(x) for x in item[1:3]))
        sig[key] += 1
        example.setdefault(key, item)
    for key, cnt in sig.most_common(100):
        print("%5d  %s" % (cnt, " | ".join(str(x) for x in example[key])))
    if len(sig) > 100:
        print("  ... %d more signatures; see the bucket file" % (len(sig) - 100))

# dump raw buckets for offline triage (grep/sort/uniq friendly)
for k, items in buckets.items():
    with open(os.path.join(SP, "bucket-%s.txt" % k), "w") as f:
        for it in items:
            f.write(" | ".join(str(x) for x in it) + "\n")
