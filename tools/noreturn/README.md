# Non-returning function lists

ParseAPI decides whether code after a call is reachable from whether the callee
returns. For a callee in another library, reached through a PLT stub, it cannot
see the callee's code, so it relies on a built-in list of names (`exit`,
`abort`, `__cxa_throw`, ...). A non-returning callee missing from that list
gets a fall-through edge into whatever follows the call, usually the next
function, which then shares blocks with the caller. In a C++ library such as
`libtorch_cpu.so` this happens at thousands of calls to error helpers like
`c10::detail::torchCheckFail`.

Tools can add names before parsing:

| API | name form |
| --- | --- |
| `BPatch::addNonReturningMangledName`, `CodeSource::addNonReturningMangledName` | the symbol name, matched exactly: `_ZN3c106detail14torchCheckFailEPKcS2_jS2_` |
| `BPatch::addNonReturningDemangledName`, `CodeSource::addNonReturningDemangledName` | with a parameter list, matched with parameters: `c10::detail::torchCheckFail(char const*, char const*, unsigned int, char const*)`; without one, every overload: `c10::detail::torchCheckFail` |

They must be called before the first binary or process is opened; afterwards
they return false and add nothing.

The tools here produce and use such lists.

## Name files

One name per line. White space around a name is removed; blank lines and lines
starting with `#` are skipped. `/dev/null` is an empty list.

## `noreturn_check <binary> <mangled-names-file> <demangled-names-file>`

Registers the names through BPatch, parses `<binary>`, and prints the number of
functions and of non-returning functions. With `NORETURN_LIST` set it also
lists the non-returning functions. The registration loop is the part to copy
into a tool of your own. Like any BPatch tool it needs `DYNINSTAPI_RT_LIB` set
to the runtime library.

## `generate_noreturn_list.sh <noreturn_scan> <target> <out dir> [<seed demangled-names file>]`

Derives a list for `<target>` from the code of the libraries it links against:

1. Orders the target's dependencies (`ldd`, `DT_NEEDED`) so that each library
   comes before the libraries that need it. glibc is skipped (`SKIP_RE`): the
   built-in list covers it.
2. Runs `noreturn_scan` on each library, the target last, with every name found
   so far registered, and adds the library's exported non-returning functions
   to the list. A library calls its own exported functions through its own PLT
   stubs too, so each library is scanned again until no new name appears.
3. Writes in `<out dir>`:

   | file | contents |
   | --- | --- |
   | `mangled.txt`, `demangled.txt` | the list, ready for `noreturn_check` or your own tool (`demangled.txt` is the seed) |
   | `review.tsv` | every exported non-returning function: mangled and demangled name, library, and why it was held back, if it was |
   | `missing.txt` | calls in the target that still fall through past the end of the calling function, by callee |
   | `<lib>.scan` | the raw `noreturn_scan` output |

`noreturn_scan <library> [<mangled-names-file> [<demangled-names-file>]]` does
one scan and prints `EXPORT`, `SUSPECT` and `MISSING` lines.

### What needs a human

The list is only as good as ParseAPI's view of each function.

- **Held-back names** (`review.tsv`, non-empty reason). A function is also
  non-returning when parsing stops early at a word the decoder cannot decode,
  so the return after it is never seen. Such a function, and every function
  whose verdict depends on it, is held back with the reason `undecodable`,
  `has-ret` (a return instruction in its symbol range) or `via <callee>`.
  Registering one of these would drop the code after every call to it.
- **`missing.txt`**. A callee here either does not return and is not in the
  list yet (add it after checking its source or declaration) or really
  returns into the next function, which is rare. Do not add these blindly.

### Example

x86_64, the `libtorch_cpu.so` of a PyTorch 2.x CPU wheel:

| | |
| --- | --- |
| generated names | 200 (`libtorch_cpu.so` 133, `libstdc++` 37, `libc10` 27, `libgomp` 4, `libgcc_s` 1) |
| held back | 4,475, nearly all AVX-512/AMX code (Sleef, MKL, NNPACK) the x86 decoder does not decode |
| still missing | 1 call, to `pytorch_qnnp_log_error`, which returns |
| run time | about 3 minutes, 18 GB peak |

The names from `libtorch_cpu.so` itself are functions that always throw:
unimplemented virtual methods (`c10d::Work::getTimeout`), backends that are
not built in (`at::UnknownQuantizer::quantize`, `*_overrideable`), and
`torch::autograd::generated::details::not_implemented`.
