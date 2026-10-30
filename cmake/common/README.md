# CMake files shared by the Dyninst repositories

This directory holds CMake files that Dyninst and its companion repositories,
such as [external-tests](https://github.com/dyninst/external-tests), use
unchanged. Dyninst has the master copy. Another repository copies the whole
directory, or the subdirectories it wants, and never edits its copy: a change
is made here and copied again.

Each subdirectory is one feature:

| Directory | What it adds |
| :--- | :--- |
| `ctest/` | `dyninst_add_test` and test requirements |
| `warnings/` | the compiler warning flags |
| `gersemi/` | targets that check and reformat the CMake code |

A repository adds the subdirectories it uses to `CMAKE_MODULE_PATH` and
includes their modules. Settings that differ between repositories, such as
which requirements its tests can name, stay outside this directory.

## `ctest/`

- `DyninstAddTest.cmake`: `dyninst_add_test`, `dyninst_add_test_executable`,
  `dyninst_test_requirement` and `dyninst_test_skip_reason`.
- `dyninst_test_skip.h`: `skip_message()`, which a test uses to report that
  it is skipped.

Including `DyninstAddTest.cmake` also creates the interface library
`dyninst_test_support`, which puts this directory on the include path. Every
executable added with the two functions links it, so a test includes
`"dyninst_test_skip.h"` by that name alone.

A test is skipped when it exits with 77, and prints why first:

```
skipped: needs the host to be aarch64
```

A test is never left out because something it needs is missing. Instead, it
is skipped, so a run's results always list it.

The repository defines its own requirements, in a file that calls
`dyninst_test_requirement` and is included after `DyninstAddTest.cmake`.
Dyninst's is `tests/cmake/DyninstTestRequirements.cmake`.

[`tests/README.md`](../../tests/README.md) describes the functions in full,
with examples, and lists Dyninst's requirements.

## `warnings/`

`DyninstWarnings.cmake` checks which of a list of warning flags the C and C++
compilers accept. It sets `SUPPORTED_C_WARNING_FLAGS` and
`SUPPORTED_CXX_WARNING_FLAGS` to them, and the configure output shows both
lists.

The repository declares three options it reads:

| Option | Default | Meaning |
| :--- | :--- | :--- |
| `DYNINST_WARNINGS_AS_ERRORS` | `OFF` | add `-Werror` |
| `DYNINST_EXTRA_WARNINGS` | empty | more flags to try |
| `DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS` | `OFF` | drop every suppression |

`DYNINST_EXTRA_WARNINGS` is a `;`-separated list without the leading `-`, as
in `Wnull-dereference;Wuseless-cast`.

In Debug builds it raises the `-Wframe-larger-than` limit for every file,
because frames are larger without optimization.

The module still holds some settings for Dyninst itself:

- Larger frame-size limits for three Dyninst source files, which Dyninst's
  own CMake files apply. Elsewhere they are unused.
- `clangBoostDeprecatedWarning`, for Clang's warnings about Boost. Elsewhere
  it is unused.
- `-Wno-unknown-pragmas`, unless Dyninst is built with OpenMP and without
  static libraries. Elsewhere it is always added.

## `gersemi/`

`DyninstCMakeFormat.cmake` adds four targets, none of them part of `all`:

| Target | What it does |
| :--- | :--- |
| `cmake-check` | lists the files that are not formatted |
| `cmake-check-raw` | the same, as gersemi prints it |
| `cmake-reformat-diff` | shows what reformatting would change |
| `cmake-reformat` | reformats the files in place |

Each runs `DyninstRunGersemi.cmake` with `cmake -P`. That script formats every
`.cmake` and `CMakeLists.txt` file that git tracks, with the settings in the
repository's `.gersemirc`. It checks that the gersemi it runs is the version
pinned in the repository's `docker/dependencies.versions`, because releases
format differently.

A missing gersemi is not a configure error, only an error for whoever builds
one of these targets. The script then explains how to install the pinned
version.

The repository provides `.gersemirc`, the pinned version, and the CI workflow
that runs the check; Dyninst's is `.github/workflows/cmake-formatting.yaml`.
