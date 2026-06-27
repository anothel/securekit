# Contributing

Use the release preflight before calling a change ready:

```sh
cmake -S . -B build -DSECUREKIT_BUILD_TESTS=ON
cmake --build build --config Release --target release-preflight
```

## One-command local check

After the build directory exists, this is the local release gate:

```sh
cmake --build build --config Release --target release-preflight
```

`release-preflight` requires `SECUREKIT_BUILD_TESTS=ON`,
`SECUREKIT_BUILD_CLI=ON`, and `SECUREKIT_INSTALL_CLI=ON`.

Do not add public API, wire formats, package channels, CI cost, or release
ceremony without a written problem, regression check, and rollback plan.

Do not expand SecureKit into TLS, networking, web middleware, custom crypto,
secure key storage, or framework-scale abstractions.
