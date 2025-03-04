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