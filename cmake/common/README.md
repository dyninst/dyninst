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

## Copying and checking

A change to these files is made in Dyninst. Once it has merged, each
repository that has a copy replaces it with Dyninst's, from the top of that
repository:

    rm -r cmake/common
    cp -r <dyninst>/cmake/common cmake/common

These repositories have a copy:

- [external-tests](https://github.com/dyninst/external-tests)

Each of them checks its copy in CI, in a job of its own, with
`DyninstCheckCopy.cmake`. The job checks out Dyninst's master and runs

    cmake -DCOPY=cmake/common -DMASTER=<dyninst>/cmake/common \
          -P cmake/common/DyninstCheckCopy.cmake

It lists any file that differs from Dyninst's, is missing, or is extra, and
fails if there is one. So it fails when the copy was edited directly, and
also after a change in Dyninst until the copy is updated. In that case it
fails on every pull request in that repository, whatever the pull request
changes; being a separate job, it is easy to tell apart from the pull
request's own results.

A new repository copies the directory and adds the job, and is added to the
list above.

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

`DyninstWarnings.cmake` checks which warning flags the C and C++ compilers
accept. It sets `SUPPORTED_C_WARNING_FLAGS` and `SUPPORTED_CXX_WARNING_FLAGS`
to them, and the configure output shows both lists.

It does not apply them. `dyninst_add_test` compiles every test with the C++
list; any other target adds them itself:

```cmake
target_compile_options(mytool PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
```

### Warning levels

The flags come in levels. Each level adds flags to the ones below it:

| Level | Adds |
| :--- | :--- |
| 0 | nothing |
| 1 | `-Wall` |
| 2 | `-Wextra` |
| 3 | `-Wpedantic` |
| 4 | 35 more |
| 5 | 5 more |

`DyninstWarningLevels.cmake` lists them. The default is `max`, the highest
level.

Levels never change once added: new warnings go into a new level. A
repository that sets its level keeps the same flags when Dyninst adds
warnings, and moves up when it is ready:

```cmake
set(REQUESTED_WARNING_LEVEL 4)
include(DyninstWarnings)
```

A developer can choose another level when configuring, with
`-DDYNINST_WARNING_LEVEL=<n>`. Left empty, the default, it uses the
repository's.

To add warnings, add a level. The steps are at the top of
`DyninstWarningLevels.cmake`.

### Options and other flags

The module declares four options:

| Option | Default | Meaning |
| :--- | :--- | :--- |
| `DYNINST_WARNING_LEVEL` | empty | the level, or the repository's |
| `DYNINST_WARNINGS_AS_ERRORS` | `OFF` | add `-Werror` |
| `DYNINST_EXTRA_WARNINGS` | empty | more flags to try |
| `DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS` | `OFF` | drop every suppression |

`DYNINST_EXTRA_WARNINGS` is a `;`-separated list without the leading `-`, as
in `Wnull-dereference;Wuseless-cast`.

A repository adds flags of its own by setting `REQUESTED_WARNING_FLAGS`
before the include, in the same form. They come after the levels' flags, so
a `-Wno-` flag among them turns off one of those. Settings for particular
files of its own also stay outside: Dyninst's are in
`cmake/DyninstWarningOverrides.cmake`, which includes this module.

### Suppressing a warning in code

Two Dyninst headers have macros for code that a warning flag objects to:
`compiler_diagnostics.h` and `compiler_annotations.h`, in Dyninst's
`common/h`. They are installed with Dyninst's other headers, so any
repository built against Dyninst can include them. Each macro expands to
what the compiler in use needs, or to nothing.

`compiler_diagnostics.h` turns one warning off for a region of code:

```c++
DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA
char buf[n];
DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA
```

Each macro goes on a line of its own, without `()` or `;`. The warnings it
covers:

| Code | Warning |
| :--- | :--- |
| `FLEX_ARRAY` | flexible array member |
| `VLA` | variable-length array |
| `VLA_EXTENSION` | Clang's VLA extension |
| `VLA_ALL` | both of the above |
| `VLA_GCC_PRAGMA_BUG` | `VLA`, older GCC only |
| `LOGICAL_OP` | GCC 6-8 false positive |
| `DUPLICATED_BRANCHES` | GCC 7-8 false positive |
| `UNUSED_VARIABLE` | Clang before 10 |
| `MAYBE_UNINITIALIZED` | GCC 12-15 false positive |

- `FLEX_ARRAY`, `VLA`: C features that C++ does not have.
- `VLA_GCC_PRAGMA_BUG`: GCC before 9, and 11.0-11.1, apply the suppression
  a line late. It expands to the `VLA` suppression on those only.
- `LOGICAL_OP`: an expression such as `x == a && x == b`, where `a` and `b`
  are different constants with the same value.
- `DUPLICATED_BRANCHES`: the same, for the conditions of an `if`/`else if`
  chain.
- `UNUSED_VARIABLE`: a variable that exists only for its constructor and
  destructor, such as a lock guard.
- `MAYBE_UNINITIALIZED`: `boost::optional::value_or`.

`DYNINST_SUPPRESS_UNUSED_VARIABLE(var)` marks one variable as meant to be
unused.

`compiler_annotations.h` has attributes that let the compiler check more,
or stop a warning that is expected:

| Macro | Goes |
| :--- | :--- |
| `DYNINST_FALLTHROUGH;` | at the end of a `case` |
| `DYNINST_DEPRECATED("msg")` | before a declaration |
| `DYNINST_PRINTF_ANNOTATION(f, a)` | after a declaration |
| `DYNINST_SCANF_ANNOTATION(f, a)` | after a declaration |
| `DYNINST_FORMAT_ANNOTATION(t, f, a)` | after a declaration |
| `DYNINST_MALLOC_ANNOTATION` | after a declaration |
| `DYNINST_MALLOC_DEALLOC_ANNOTATION(d)` | after a declaration |
| `DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(d, p)` | after a declaration |

- `DYNINST_FALLTHROUGH`: the `case` falls through to the next one on
  purpose.
- `DYNINST_DEPRECATED`: a use of the function, variable or type warns,
  with the message.
- `DYNINST_PRINTF_ANNOTATION`, `DYNINST_SCANF_ANNOTATION`: the function
  takes a `printf` or `scanf` format string, at 1-based argument `f`, and
  its values from argument `a` on, so the compiler checks them:

  ```c++
  void log(int level, const char *fmt, ...) DYNINST_PRINTF_ANNOTATION(2, 3);
  ```

  `DYNINST_FORMAT_ANNOTATION` is the same with the format type `t` given.
- `DYNINST_MALLOC_*`: the function returns newly allocated memory;
  `d` names the function that frees it, and `p` that function's argument
  that takes the pointer.

Each header's comments describe its macros in full. Defining
`DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS` makes every `compiler_diagnostics.h`
macro expand to nothing, so the warnings appear. `DyninstWarnings.cmake`
defines it for every target when `DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS` is
on.

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
