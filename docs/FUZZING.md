# Fuzzing

SecureKit fuzz targets are optional and require Clang with libFuzzer support.
They do not build in the default release or package path.

```sh
cmake -S . -B build-fuzz \
  -DSECUREKIT_BUILD_FUZZ=ON \
  -DSECUREKIT_BUILD_TESTS=OFF \
  -DSECUREKIT_BUILD_CLI=OFF \
  -DSECUREKIT_INSTALL_CLI=OFF \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build build-fuzz --target fuzz-smoke
```

`fuzz-smoke` runs each target once against `tests/fixtures` plus malformed
samples under `tests/fuzz/corpus`. Longer fuzzing remains a manual or scheduled
job:

```sh
build-fuzz/securekit_fuzz_hex tests/fixtures tests/fuzz/corpus
build-fuzz/securekit_fuzz_base64 tests/fixtures tests/fuzz/corpus
build-fuzz/securekit_fuzz_base64url tests/fixtures tests/fuzz/corpus
build-fuzz/securekit_fuzz_skt1 tests/fixtures tests/fuzz/corpus
build-fuzz/securekit_fuzz_file tests/fixtures tests/fuzz/corpus
```

## Corpus Policy

- Keep checked-in corpus entries small, stable, and format-focused.
- Put byte-format compatibility samples in `tests/fixtures`; put malformed
  smoke-only seeds in `tests/fuzz/corpus`.
- Add a corpus seed only when it exercises a parser path not already covered by
  fixtures or unit tests.
- Minimized crash reproducers may be checked in after the root cause is fixed
  and a regression test names the protected rule.
- Do not check in generated fuzz output, large random corpora, secrets, or
  environment-specific files.

## Scheduled Job Policy

Scheduled fuzzing is owned by the maintainer who owns release readiness for the
target branch. It must run the existing `fuzz-smoke` target first; longer runs
are promoted only after repeated useful local signal from the same targets.

Runtime limit: each scheduled target gets a fixed wall-clock limit, recorded in
the workflow or scheduler config, and the whole job must time out instead of
running indefinitely.

Artifact policy: keep only minimized crash inputs, sanitizer logs, target name,
commit SHA, compiler version, and command line. Do not upload generated corpora,
large random inputs, passwords, keys, plaintext samples, or environment-specific
build trees.

Failure triage path:

1. Reproduce the crash locally with the saved input and exact target.
2. Classify the failure as parser bug, expected rejection, sanitizer-only
   finding, infrastructure failure, or duplicate.
3. Add or update a focused unit test, negative fixture, or corpus seed.
4. Fix the root cause, rerun `fuzz-smoke`, and keep only the minimized
   reproducer when it protects a parser path not already covered.
