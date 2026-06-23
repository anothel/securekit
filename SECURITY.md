# Security Policy

## Reporting A Vulnerability

Do not report security vulnerabilities through public GitHub issues.

Preferred path: open a private report through GitHub Security Advisories:
<https://github.com/anothel/securekit/security/advisories/new>.

If that path is unavailable, use a private contact method listed on the
maintainer GitHub profile: <https://github.com/anothel>. Do not publish details
until a fixed release or advisory is ready.

Include enough information to reproduce and classify the issue:

- SecureKit version, tag, or commit
- OS, compiler, CMake, and OpenSSL versions
- OpenSSL provider configuration if relevant
- minimal reproducer
- expected behavior
- actual behavior
- affected API, CLI command, or serialized format
- whether plaintext, keys, passwords, or authenticated data may be exposed
- exploitability assumptions

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
upgrade instructions, and whether existing `SKT1`, `SKF1`, or `SKP1` data needs
to be regenerated.
