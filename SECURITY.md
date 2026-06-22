# Security Policy

## Reporting A Vulnerability

Do not report security vulnerabilities through public GitHub issues.

Use GitHub Security Advisories when available, or contact the maintainer
privately before publishing details. Include enough information to reproduce and
classify the issue:

- affected SecureKit version or commit
- operating system, compiler, and CMake version
- OpenSSL version and provider configuration if relevant
- minimal reproduction steps
- expected behavior
- actual behavior
- whether plaintext, keys, passwords, or authenticated data may be exposed

## Supported Versions

Security fixes are provided for the latest released version unless the release
notes state otherwise. Pre-release commits on `main` are supported on a
best-effort basis until the next release is cut.

## Disclosure

The maintainer will coordinate disclosure timing after confirming the issue,
preparing a fix, and deciding whether existing releases are affected. Avoid
sharing exploit details publicly before a fixed release or advisory is ready.

## Scope

In scope:

- authentication bypass
- plaintext exposure caused by SecureKit
- key, nonce, or AAD misuse caused by SecureKit
- unsafe output-file behavior
- parser crashes or memory safety issues caused by malformed SecureKit packets
  or files
- misleading security documentation that could cause unsafe use

Out of scope:

- vulnerabilities in caller code
- insecure storage or transmission of caller-provided keys or passwords
- compromised operating systems, filesystems, shells, or build hosts
- unsupported OpenSSL versions
- misuse of APIs after documented preconditions are ignored

## Security Release Notes

Security-impacting releases should mention the affected surface, fixed version,
upgrade guidance, and whether existing `SKT1`, `SKF1`, or `SKP1` data needs to
be regenerated.
