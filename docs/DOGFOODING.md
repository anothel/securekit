# SecureKit Dogfooding

Dogfooding checks one release-like package as a user would consume it, without
adding new public surface.

Run:

```sh
cmake --build build --config Release --target dogfood-check
```

The check:

- extracts one binary archive from `package-check` artifacts;
- runs `keygen`, `seal-file`, `verify-file`, and `open-file`;
- runs the password file flow with `seal-file-password`,
  `verify-file-password`, and `open-file-password`;
- configures, builds, and runs a copied C++ consumer from the extracted
  package's installed CMake config.

## v0.2.0 local dogfood

Result: no repeated friction recorded.

This run did not require extra public surface. Follow-up work now lives in
`docs/ROADMAP.md` under `Fix Queue`, with a named check for each item.
