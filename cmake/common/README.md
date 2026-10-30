# CMake files shared by the Dyninst repositories

This directory holds CMake files that Dyninst and its companion repositories,
such as [external-tests](https://github.com/dyninst/external-tests), use
unchanged. Dyninst has the master copy. Another repository copies the whole
directory and never edits its copy: a change is made here and copied again.

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

### Adding a test

Every test is added with `dyninst_add_test`. It always creates the test:
if the test cannot be built or run in this configuration, because an
option is off or a tool is missing, ctest reports it as skipped and says
why. Tests are written in C++.

    dyninst_add_test(symtab_parser
      SOURCE_FILES parser.cpp
      DEPS Dyninst::symtabAPI
      REQUIRES static_exe
    )

This builds an executable named after the test and runs it. A checker
that several tests run with different arguments is made once with
`dyninst_add_test_executable` and named in each test's `COMMAND`:

    dyninst_add_test_executable(getSOName
      SOURCE_FILES getSOName.cpp
      DEPS Dyninst::symtabAPI
    )
    dyninst_add_test(symtab_soname_absent
      COMMAND getSOName $<TARGET_FILE:soname_absent> ""
    )

### dyninst_add_test

    dyninst_add_test(<TestName>
      [TYPE <type>]
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
| `TYPE`            | one of `DYNINST_TEST_TYPES`         | if that is set          |
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
  `ctest -L unit` selects it. A project with test types lists them in
  `DYNINST_TEST_TYPES` before adding any test; every test then names
  one. Without `DYNINST_TEST_TYPES`, `TYPE` is an error.

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
  PRIVATE: in Dyninst's tree a library's own target, such as
  `symtabAPI`, and elsewhere the installed one, such as
  `Dyninst::symtabAPI`.

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

| Property           | Value                                         |
| :----------------- | :-------------------------------------------- |
| `LABELS`           | `TYPE`, and `host` with a `host:` requirement |
| `SKIP_RETURN_CODE` | `77`                                          |

Arguments in `COMMAND` and `ARGS` are passed as written: an empty
argument (`""`) is kept, and so is a `;` inside an argument.

It is a configuration error to leave out `TYPE` or give one not in
`DYNINST_TEST_TYPES` when that is set, to give `TYPE` when it is not, a
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
puts `ctest/` on the include path for `dyninst_test_skip.h`.

### Requirements

A requirement is a condition, known when CMake runs, that a test needs.
The project defines its own, in a file that includes
`DyninstAddTest.cmake` and is included before any test is added;
Dyninst's is `tests/cmake/DyninstTestRequirements.cmake`, and
[`tests/README.md`](https://github.com/dyninst/dyninst/blob/master/tests/README.md)
lists them.

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
directory's variables.

A name of the form `<prefix>:<arg>` defines a family. For Dyninst's
`codegen:<arg>`, `REQUIRES codegen:x86_64` puts `x86_64` in place of
`<arg>`, and `X86_64` in place of `<ARG>`, in the condition and the
reason. A family must list the arguments it accepts in `VALUES`, and any
other is a configuration error, so a misspelled one cannot make a test
that is always skipped. A plain name is looked up before a family, so
one member of a family can have a definition of its own.

A family's argument can also be a comma-separated list, met when any one
of its values is: `host:aarch64,ppc64le` is met on either host, and the
reason reads "needs the host to be aarch64 or ppc64le". Each value is
checked against `VALUES`. A member with a definition of its own, such as
`codegen:i386`, uses it inside a list too. A test that needs all of
several values names each separately.

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
`ctest/dyninst_test_skip.h`, which every test can include:

    return skip_message("HOME is not set");

A test that sets up a fixture (`FIXTURES_SETUP`) must not skip itself
when it runs: ctest counts a skipped setup test as passed and runs the
tests that require it. Give every test of a fixture the same
requirements instead, so that they skip together.

### Output

ctest records stdout and stderr together, so the choice does not
change a result, but it matters when a test is run by hand, and
stderr is unbuffered: what is written to it survives a crash and
keeps its place among the other messages.

- **stdout:** The result only: the skip line from `skip_message()`, which
  flushes it, and at most one final summary line, such as `failed =
  false`.

- **stderr:** Everything else: what failed and why (what was expected and
  what was seen), reports of each case, progress, and usage errors.

## `warnings/`

`DyninstWarnings.cmake` checks which of a list of warning flags the C and C++
compilers accept. It sets `SUPPORTED_C_WARNING_FLAGS` and
`SUPPORTED_CXX_WARNING_FLAGS` to them, and the configure output shows both
lists. `dyninst_add_test` compiles every test with the C++ list.

It declares three options:

| Option | Default | Meaning |
| :--- | :--- | :--- |
| `DYNINST_WARNINGS_AS_ERRORS` | `OFF` | add `-Werror` |
| `DYNINST_EXTRA_WARNINGS` | empty | more flags to try |
| `DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS` | `OFF` | drop every suppression |

`DYNINST_EXTRA_WARNINGS` is a `;`-separated list without the leading `-`, as
in `Wnull-dereference;Wuseless-cast`.

A repository adds flags of its own by setting `REQUESTED_WARNING_FLAGS`
before the include, in the same form. They come after the module's own
flags, so a `-Wno-` flag among them wins. Settings for particular files
of its own also stay outside: Dyninst's are in
`cmake/DyninstWarningOverrides.cmake`, which includes this module.

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
repository's `.gersemirc`.

Releases of gersemi format differently, so a repository pins one. It sets
`DYNINST_GERSEMI_VERSIONS_FILE` before the include to a file holding a line
`gersemi:<version>`; Dyninst's is `docker/dependencies.versions`. The script
then refuses any other version, and its install instructions name the pinned
one. Without the setting, any gersemi is accepted.

A missing gersemi is not a configure error, only an error for whoever builds
one of these targets. The script then explains how to install it.

The repository provides `.gersemirc`, the pin, and the CI workflow that runs
the check. A workflow can run the script directly, without configuring:

    cmake -DGERSEMI=$(command -v gersemi) -DMODE=check -DSOURCE_DIR=$PWD \
          -DVERSIONS_FILE=docker/dependencies.versions \
          -P cmake/common/gersemi/DyninstRunGersemi.cmake

`MODE` is `check`, `check-raw`, `diff` or `in-place`, and a relative
`VERSIONS_FILE` is taken from `SOURCE_DIR`. Dyninst's workflow is
`.github/workflows/cmake-formatting.yaml`.
