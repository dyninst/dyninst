# x86 decoder verification tools

Developer tools used for the decode-table audit documented in
`docs/x86-decode-fixes/`. They are not part of the build; compile them
by hand against an existing build tree.

## decode_dump.cpp — inspect individual encodings

Decodes hex byte sequences and prints the disassembly, entry ID, length,
register read/write sets, memory access direction, and whether a
control-flow target exists — everything needed to compare one encoding
against its SDM page.

```console
$ g++ -std=c++11 -o decode_dump \
      tests/integration/InstructionAPI/decoder/x86/tools/decode_dump.cpp \
      -IinstructionAPI/h -Icommon/h -Ibuild/common/h \
      -Lbuild/instructionAPI -Lbuild/common -linstructionAPI -lcommon \
      -Wl,-rpath,$PWD/build/instructionAPI -Wl,-rpath,$PWD/build/common
$ ./decode_dump "f3 0f ae 37" "0f 01 f9"
f3 0f ae 37    | 'clrssbsy (%rdi)' id=... len=4 R:{RDI } W:{} memR=1 memW=1 cft=0
0f 01 f9       | 'rdtscp' id=... len=3 R:{} W:{EAX EDX ECX } memR=0 memW=0 cft=0
$ ./decode_dump -32 "f3 0f ae c0"     # decode for 32-bit mode
f3 0f ae c0    | INVALID
```

Cross-check the same bytes with objdump:

```console
$ printf '\xf3\x0f\xae\x37' > /tmp/x.bin
$ objdump -D -b binary -m i386:x86-64 /tmp/x.bin
```

## sweep.cpp + gen_and_compare.py — differential sweep vs objdump

`gen_and_compare.py` generates ~45k encodings (all one-/two-byte
opcodes with all mandatory prefixes and ModRM.reg values, the 0F 38 /
0F 3A maps, and the VEX3 space), writes them into fixed-size slots of a
blob, decodes every slot with both `sweep` (Dyninst) and objdump, and
buckets the disagreements. See the module docstring for the bucket
meanings and the caveat that **neither decoder is ground truth** — every
actionable disagreement must be verified against the Intel SDM or AMD
APM before a table is changed.

```console
$ g++ -std=c++11 -O1 -o tests/integration/InstructionAPI/decoder/x86/tools/sweep \
      tests/integration/InstructionAPI/decoder/x86/tools/sweep.cpp \
      -IinstructionAPI/h -Icommon/h -Ibuild/common/h \
      -Lbuild/instructionAPI -Lbuild/common -linstructionAPI -lcommon \
      -Wl,-rpath,$PWD/build/instructionAPI -Wl,-rpath,$PWD/build/common
$ python3 tests/integration/InstructionAPI/decoder/x86/tools/gen_and_compare.py
```

Outputs a per-bucket summary on stdout plus raw `bucket-*.txt` files
(written next to the script) for grep/sort triage, e.g.:

```console
$ sed -E 's/-r[0-9]-/-r*-/' bucket-mnemonic-mismatch.txt \
    | awk -F' \\| ' '{split($3,a," "); print $1" dyn="$2" od="a[1]}' \
    | sort | uniq -c | sort -rn | head
```

Triage tips:

* `length-mismatch` is the highest-priority bucket: a length
  disagreement desynchronizes stream parsing from that point on.
* `mnemonic-mismatch` mixes real identity bugs with objdump naming
  conventions (AT&T size suffixes, cc-aliases, imm-specialized
  pseudo-ops); the benign patterns are filtered by the `ALIAS` table in
  the script — extend it rather than ignoring a bucket line.
* `objdump-rejects` is usually Dyninst being permissive about reserved
  prefixed encodings or mod-form constraints; `dyninst-rejects` is
  usually a missing instruction.
* The sweep only checks identity and length. Read/write-set and operand
  errors (like the group 9 `xsavec` legacy-type bug) need the manual
  table-vs-SDM comparison plus `decode_dump`.
