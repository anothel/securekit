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
