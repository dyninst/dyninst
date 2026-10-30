# Unit, integration, and regression tests for Dyninst

These tests are separate from the [test suite](https://github.com/dyninst/testsuite) and the [external tests](https://github.com/dyninst/external-tests) in that they are designed to be run quickly from the build tree at arbitrary times (e.g., while you are still working on code changes).

## Unit tests

These should test a **single** function or class. They should be as small as possible and run in less than a second; preferably in a few tens of milliseconds. Unit tests should not depend on external state (e.g., reading from a file).

## Integration tests

These should target a single function, but can be part of a larger sequence of function calls. For example, testing the `InstructionDecoder` implicitly uses the decoding backend, but still targets a single function. Like unit tests, they should not depend on external state.

## Regression tests

These can be complex tests that require a few seconds to run and exercise complex code paths. They should still run in under a second to keep testing reasonable. They can use external files. However, we do not want to keep binary files in the Dyninst source tree. They should be stored elsewhere. Including source files that are compiled into a mutatee is fine, though.


---

Tests are enabled with `DYNINST_ENABLE_TESTS=VALUE` where `VALUE` determines which type(s) of tests are available:

| `VALUE` | Meaning |
|--------|--------|
| OFF | Disable all tests |
| ON | Enable unit tests only |
| UNIT | Same as `ON` |
| INTEGRATION | Enable integration and unit tests |
| REGRESSION | Enable regression, integration, and unit tests| 
| ALL | Enable all tests | 

When more than one type of test is enabled (e.g., `VALUE=ALL`), each type can be run separately using `ctest -L [unit,integration,regression]`. See the ctest [docs](https://cmake.org/cmake/help/latest/manual/ctest.1.html#cmdoption-ctest-L) for details on labels.

## Adding a test

Every test is added with `dyninst_add_test`, from
`cmake/common/ctest/DyninstAddTest.cmake`, which the external tests
share. [`cmake/common/README.md`](../cmake/common/README.md) describes
it in full: its keywords, defining a requirement, skipping, and where a
test's output goes. It always creates the test: if the test cannot be
built or run in this configuration, ctest reports it as skipped and says
why. Only `DYNINST_ENABLE_TESTS` decides which tests exist, so every
build of a test type has the same tests.

    dyninst_add_test(parseAPI_parseGaps_test
      TYPE regression
      SOURCE_FILES parseGaps.cpp
      DEPS parseAPI
      ARGS
        "${DYNINST_TEST_BINARIES_DIR}/parseAPI/parseGaps/aarch64_el0_stripped"
      REQUIRES test_binaries
    )

Every test here names its type in `TYPE`: `unit`, `integration` or
`regression`, the type of the directory it is in. It becomes the test's
label, so `ctest -L unit` selects it. `tests/CMakeLists.txt` lists the
types in `DYNINST_TEST_TYPES`. `DEPS` names Dyninst's libraries by their
targets in the tree, such as `parseAPI`.

### Requirements

A requirement is a condition, known when CMake runs, that a test needs.
They are defined in `tests/cmake/DyninstTestRequirements.cmake`, each
with the reason a test prints when it is not met:

| Requirement              | Met when                               |
| :----------------------- | :------------------------------------- |
| `test_binaries`          | `DYNINST_ENABLE_TESTS=ALL`             |
| `capstone`               | `DYNINST_ENABLE_CAPSTONE`              |
| `objdump`                | objdump was found                      |
| `static_exe`             | the toolchain links static executables |
| `static_lib:<lib>`       | `<lib>_static` is built                |
| `codegen:<arch>`         | code generation for `<arch>`           |
| `host:<arch>`            | the host is `<arch>`                   |
| `native`                 | code generation for the host           |
| `dynamic`                | `DYNINST_ENABLE_DYNAMIC_TESTS`         |
| `userns`                 | `DYNINST_ENABLE_USERNS_TESTS`          |
| `stackanalysis_any_arch` | not a native aarch64 build             |

- `test_binaries`: The test_binaries repository, which
  `DYNINST_ENABLE_TESTS=ALL` downloads; its path is
  `${DYNINST_TEST_BINARIES_DIR}`.

- `static_lib:<lib>`: `<lib>` is any Dyninst library.
  `ENABLE_STATIC_LIBS` builds the static ones.

- A family such as `codegen:<arch>` or `host:<arch>` also takes a
  comma-separated list, met when any one of its values is:
  `host:aarch64,ppc64le` is met on either host, and a test that is not
  run reports `skipped: needs the host to be aarch64 or ppc64le`. A
  test that needs all of several values names each separately.

- `codegen:<arch>`: `<arch>` is one of `DYNINST_CODEGEN_ARCHES`:
  `x86_64`, `i386`, `aarch64`, `ppc64le`, `riscv64`, `amdgpu_gfx908`,
  `amdgpu_gfx90a`, `amdgpu_gfx940` or `amdgpu_gfx950`. `codegen:i386` is
  also met for x86_64, which builds the IA-32 emitter too.

- `host:<arch>`: The test runs code written for one architecture, such
  as assembly. `<arch>` is one of `DYNINST_HOST_ARCHES`: `x86_64`,
  `i386`, `aarch64`, `ppc64le` or `riscv64`. Use it only when the
  program the test runs, or the Dyninst code it tests, works on that
  host alone. A test that reads binaries does not need it: Symtab and
  ParseAPI read every architecture on any host, so give it binaries for
  each architecture instead. Tests with a `host:` requirement are also
  labeled `host`, so `ctest -N -L host` lists them.

- `native`: The test runs a program that Dyninst instruments or
  rewrites, which works only when Dyninst generates code for the host.
  It is not met when `DYNINST_CODEGEN_ARCH` names another architecture,
  as in the codegen cross-build CI jobs, which build for aarch64,
  ppc64le, i386 and amdgpu on an x86_64 host. `dynamic` does not imply
  it: a test can attach with ptrace without generating code. A test
  whose mutatee is x86_64 assembly needs `host:x86_64 native`.

- `stackanalysis_any_arch`: StackAnalysis takes its entry state from the
  build's code-generation architecture, and a native aarch64 build has none
  for the code of another architecture. An aarch64 build on an x86_64 host
  uses x86's entry state.

- `dynamic` and `userns`: The test drives a live process with ptrace
  (`dynamic`), or creates unprivileged user namespaces with `unshare()`
  (`userns`). Both options are on by default. Turn one off only on a
  host known to lack the feature: for `dynamic`, a container without
  `CAP_SYS_PTRACE` or a host with yama `ptrace_scope=3`. While one is on,
  a test that cannot use the feature fails instead of skipping, so a
  host whose security settings changed is noticed. On Ubuntu 23.10 and
  later, AppArmor (`kernel.apparmor_restrict_unprivileged_userns=1`, the
  default) allows unprivileged user namespaces but gives no capabilities
  in them; the one test that needs a capability there, the chroot case
  of `common_filesystem`, then reports itself as skipped and names that
  setting.

A new requirement is defined in that file with
`dyninst_test_requirement`, described in
[`cmake/common/README.md`](../cmake/common/README.md).
