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
`tests/cmake/DyninstAddTest.cmake`. It always creates the test: if the
test cannot be built or run in this configuration, because an option is
off or a tool is missing, ctest reports it as skipped and says why. Only
`DYNINST_ENABLE_TESTS` decides which tests exist, so every build of a
test type has the same tests. Tests are written in C++.

    dyninst_add_test(parseAPI_parseGaps_test
      TYPE regression
      SOURCE_FILES parseGaps.cpp
      DEPS parseAPI
      ARGS
        "${DYNINST_TEST_BINARIES_DIR}/parseAPI/parseGaps/aarch64_el0_stripped"
      REQUIRES test_binaries
    )

This builds an executable named after the test and runs it. A checker
that several tests run with different arguments is made once with
`dyninst_add_test_executable` and named in each test's `COMMAND`:

    dyninst_add_test_executable(getSOName
      SOURCE_FILES getSOName.cpp
      DEPS symtabAPI
    )
    dyninst_add_test(symtab_soname_absent
      TYPE regression
      COMMAND getSOName $<TARGET_FILE:soname_absent> ""
    )

### dyninst_add_test

    dyninst_add_test(<TestName>
      TYPE unit|integration|regression
      SOURCE_FILES <file>... | COMMAND <command> [<arg>...]
      [ARGS <arg>...]
      [DEPS <target>...]
      [DEFINES <val>...]
      [COMPILE_OPTIONS <option>...]
      [REQUIRES <requirement>...]
      [PROPERTIES <property> <value>...]
    )

| Keyword           | Takes                               | Required                |
| :---------------- | :---------------------------------- | :---------------------- |
| `<TestName>`      | the test's name                     | yes                     |
| `TYPE`            | `unit`, `integration`, `regression` | yes                     |
| `SOURCE_FILES`    | source files                        | this or `COMMAND`       |
| `COMMAND`         | a command and its arguments         | this or `SOURCE_FILES`  |
| `ARGS`            | arguments                           | no; `SOURCE_FILES` only |
| `DEPS`            | targets                             | no; `SOURCE_FILES` only |
| `DEFINES`         | compile definitions                 | no; `SOURCE_FILES` only |
| `COMPILE_OPTIONS` | compile options                     | no; `SOURCE_FILES` only |
| `REQUIRES`        | requirement names                   | no                      |
| `PROPERTIES`      | property name/value pairs           | no                      |

- `<TestName>`: The name ctest lists. With `SOURCE_FILES` it is also the
  name of the executable target.

- `TYPE`: The test type, which becomes the test's `LABELS`, so that
  `ctest -L unit` selects it. Use the type of the directory the test is
  in: whether a type's directory is added at all is decided by
  `DYNINST_ENABLE_TESTS`.

- `SOURCE_FILES`: Builds an executable named `<TestName>` from these
  files and runs it with `ARGS`. Nothing is built if a requirement is
  not met.

- `COMMAND`: Runs this command instead, with its arguments. It is
  usually a target made by `dyninst_add_test_executable`, named directly
  or as `$<TARGET_FILE:...>`; if that target was skipped for an unmet
  requirement, so is the test, with the same reason. It can also be any
  other command, such as `${CMAKE_COMMAND} -E compare_files ...` or
  `${CMAKE_COMMAND} -P <script>`.

- `ARGS`: The arguments for the executable built from `SOURCE_FILES`.

- `DEPS`: Libraries and other targets the executable links, always
  PRIVATE.

- `DEFINES`: PRIVATE compile definitions, for example
  `${DYNINST_PLATFORM_CAPABILITIES}` to get the library's platform
  macros, such as `os_linux`.

- `COMPILE_OPTIONS`: PRIVATE compile options for every source. C++
  sources are already compiled with `SUPPORTED_CXX_WARNING_FLAGS`, which
  come first, so an option here, such as `-Wframe-larger-than=50000`,
  can relax one.

- `REQUIRES`: Requirements the test needs (see Requirements below). If
  any is not met, nothing is built for the test, and it runs a
  placeholder that prints `skipped: ` and every unmet reason, joined
  with `, `, and exits with 77.

- `PROPERTIES`: Test properties, passed to `set_tests_properties`, such
  as `TIMEOUT 5`, `WILL_FAIL TRUE`, `ENVIRONMENT "A=1;B=2"`,
  `PASS_REGULAR_EXPRESSION` or `FIXTURES_SETUP`. CMake does not check
  the names or values: a misspelled property is silently ignored. A
  skipped test keeps them; exit code 77 takes precedence over
  `WILL_FAIL` and the pass and fail patterns.

Every test also gets these properties; do not set them yourself:

| Property           | Value  |
| :----------------- | :----- |
| `LABELS`           | `TYPE` |
| `SKIP_RETURN_CODE` | `77`   |

Arguments in `COMMAND` and `ARGS` are passed as written: an empty
argument (`""`) is kept, and so is a `;` inside an argument.

It is a configuration error to give a `TYPE` other than the three, a
requirement that is not defined, both or neither of `SOURCE_FILES` and
`COMMAND`, `ARGS`, `DEPS`, `DEFINES` or `COMPILE_OPTIONS` with
`COMMAND`, or an argument that follows no keyword.

### dyninst_add_test_executable

    dyninst_add_test_executable(<TargetName>
      SOURCE_FILES <file>...
      [DEPS <target>...]
      [DEFINES <val>...]
      [COMPILE_OPTIONS <option>...]
      [REQUIRES <requirement>...]
    )

| Keyword           | Takes               | Required |
| :---------------- | :------------------ | :------- |
| `<TargetName>`    | the target's name   | yes      |
| `SOURCE_FILES`    | source files        | yes      |
| `DEPS`            | targets             | no       |
| `DEFINES`         | compile definitions | no       |
| `COMPILE_OPTIONS` | compile options     | no       |
| `REQUIRES`        | requirement names   | no       |

Makes an executable for one or more tests to run, or one that a test
examines as its input. The keywords mean what they do for
`dyninst_add_test`. If a requirement is not met, the target is not
created, and a test whose `COMMAND` runs it is skipped. Every test
executable, from either function, links `dyninst_test_support`, which
puts `tests/` on the include path for `dyninst_test_skip.h`.

### Requirements

A requirement is a condition, known when CMake runs, that a test needs.
They are defined in `tests/cmake/DyninstTestRequirements.cmake`, each
with the reason a test prints when it is not met:

| Requirement        | Met when                               |
| :----------------- | :------------------------------------- |
| `test_binaries`    | `DYNINST_ENABLE_TESTS=ALL`             |
| `capstone`         | `DYNINST_ENABLE_CAPSTONE`              |
| `objdump`          | objdump was found                      |
| `static_exe`       | the toolchain links static executables |
| `static_lib:<lib>` | `<lib>_static` is built                |
| `codegen:<arch>`   | code generation for `<arch>`           |
| `dynamic`          | `DYNINST_ENABLE_DYNAMIC_TESTS`         |
| `userns`           | `DYNINST_ENABLE_USERNS_TESTS`          |

- `test_binaries`: The test_binaries repository, which
  `DYNINST_ENABLE_TESTS=ALL` downloads; its path is
  `${DYNINST_TEST_BINARIES_DIR}`.

- `static_lib:<lib>`: `<lib>` is any Dyninst library.
  `ENABLE_STATIC_LIBS` builds the static ones.

- `codegen:<arch>`: As in `DYNINST_CODEGEN_ARCH_<ARCH>`: `x86_64`,
  `i386`, `aarch64`, `ppc64le`, `riscv64`, `amdgpu_gfx908`,
  `amdgpu_gfx90a`, `amdgpu_gfx940` or `amdgpu_gfx950`.

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

A new requirement is defined with

    dyninst_test_requirement(<name> WHEN <condition>... REASON <text>
                             [VALUES <value>...])

| Keyword  | Takes                    | Required          |
| :------- | :----------------------- | :---------------- |
| `<name>` | the requirement's name   | yes               |
| `WHEN`   | an `if()` condition      | yes               |
| `REASON` | the text a test prints   | yes               |
| `VALUES` | the arguments it accepts | for a family only |

The condition is evaluated where the test is added, so it can use that
directory's variables. A name of the form `<prefix>:<arg>` defines a
family: `REQUIRES codegen:x86_64` puts `x86_64` in place of `<arg>`, and
`X86_64` in place of `<ARG>`, in the condition and the reason. A family
must list the arguments it accepts in `VALUES`, and any other is a
configuration error, so a misspelled one cannot make a test that is
always skipped. A plain name is looked up before a family, so one member
of a family can have a definition of its own.

To build something a test uses but does not run only when it can be,
ask for the reasons directly:

    dyninst_test_skip_reason(<out-var> REQUIRES <requirement>...)

This sets `<out-var>` to the unmet reasons, joined with `, `, or to an
empty string when all are met.

### Skipping

A test that cannot run prints `skipped: <reason>` and exits with 77.
Every test is registered with `SKIP_RETURN_CODE 77`, so ctest reports
it as Skipped; the exit code alone decides that. The reason appears in
`ctest -V` and in `Testing/Temporary/LastTest.log`.

There are two ways to skip:

- **When the configuration decides it:** Name the requirement in `REQUIRES`.
  If it is not met, nothing is built for the test, and it runs a
  placeholder that prints the reason and exits with 77.

- **When only the running test can tell:** For example, the CPU or the C
  library version. The test checks, prints `skipped: <reason>`, and
  exits with 77. Prefer this when the test can be built anyway, so it
  keeps being compiled.

A test does both with `skip_message()` from
`tests/dyninst_test_skip.h`, which every test can include:

    return skip_message("HOME is not set");

A test that sets up a fixture (`FIXTURES_SETUP`) must not skip itself
when it runs: ctest counts a skipped setup test as passed and runs the
tests that require it. Give every test of a fixture the same
requirements instead, so that they skip together.

