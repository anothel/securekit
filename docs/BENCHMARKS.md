# SecureKit Benchmarks

SecureKit benchmarks are local smoke measurements. They prove that the crypto
and file paths still build, run, and report repeatable line labels. They are not a performance guarantee.

Run them from a Release build:

```sh
cmake --build build --config Release --target benchmarks-check
```

Current measurements:

- `sha256_64k`: SHA-256 over a 64 KiB buffer.
- `aead_roundtrip_64k`: `SKT1` encrypt/decrypt round trip over a 64 KiB buffer.
- `file_roundtrip_2m`: `SKF1` seal/open round trip over a 2 MiB file.

Each line prints `iterations`, total `bytes`, elapsed `seconds`, and a `guard`
value that keeps benchmark work observable.
