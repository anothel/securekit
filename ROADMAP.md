# SecureKit Roadmap

This file tracks forward-looking work only. Completed migration and handoff
notes belong in Git history, not in this roadmap.

## Current Direction

- Keep the v1 free-function API stable.
- Keep OpenSSL as the crypto backend.
- Keep package consumption simple through CMake install/export and the CLI.
- Add compatibility vectors and hardening tests when new risk appears.
- Avoid broad abstractions until real call sites justify them.

## Near-Term

- Add focused known-answer vectors for areas that gain new behavior or bug fixes.
- Continue security hardening around file, packet, and CLI edge cases.
- Keep CI/package checks aligned with every public surface area.
- Keep README examples and format documentation current.

## Later

- Add object-oriented APIs only if repeated call sites need them.
- Add non-throwing APIs only if callers need explicit error-return flows.
- Add additional password formats only if KDF agility or dependency changes justify them.
- Add additional streaming APIs only if real incremental formats appear.

## Not Planned

- Custom crypto primitives.
- Custom string classes or allocators.
- TLS or networking.
- Secure key storage.
- Guaranteed memory erasure.
- Framework-scale abstractions without call-site pressure.
