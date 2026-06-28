# SecureKit Coverage Reporting

Coverage is observational. It is not a release gate until the project has a
stable baseline and a clear owner for threshold changes.

## Local Report

Requirements:

- GCC or Clang
- `gcovr`
- tests enabled

```sh
cmake -S . -B build-coverage -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSECUREKIT_BUILD_TESTS=ON \
  -DSECUREKIT_ENABLE_COVERAGE=ON
cmake --build build-coverage --target coverage-report
```

The target runs the test suite through `check` first, then writes:

- `build-coverage/coverage/securekit.html`
- `build-coverage/coverage/coverage.xml`

Use coverage to find untested SecureKit library branches. Do not block changes
on a percentage alone; add focused tests for real behavior, format, CLI, or
release risks.
