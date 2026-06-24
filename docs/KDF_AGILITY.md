# SecureKit KDF Agility Policy

SecureKit v1 has one password-file KDF profile:

| Format | Version | KDF ID | Parameters |
| --- | --- | ---: | --- |
| `SKP1` | `0x01` | `0x01` | OpenSSL scrypt, `N=32768`, `r=8`, `p=1`, `maxmem=64 MiB` |

`SKP1` readers reject any other KDF ID or scrypt parameter set. They must not
silently reinterpret stored parameters, upgrade files during read, or fall back
to weaker settings after a failed stronger profile attempt.

## Downgrade Policy

New password-file KDF behavior must use a new explicit format revision or magic
value. Reusing `SKP1` version `0x01` for different derivation behavior is not
allowed.

Readers must fail closed on unsupported KDF profiles. A file that declares an
unknown profile must return the existing malformed/unsupported file error path,
not retry with `KDF ID 0x01`, ignore parameters, or expose which secret input was
wrong.

Writers must not emit a new profile until all of these exist in the same change:

- format specification for the profile ID, salt fields, parameters, and AAD
  binding;
- downgrade behavior for old readers and new readers;
- memory and time upper bounds;
- compatibility fixtures and inventory docs.

## Bounds

Every accepted profile must define:

- maximum memory accepted by the implementation;
- expected interactive runtime target for supported platforms;
- maximum encoded parameter values accepted on read;
- failure behavior when OpenSSL or the platform cannot satisfy the profile.

The current `SKP1` profile keeps `maxmem=64 MiB`. Future profiles may choose a
different bound, but the bound must be fixed in the format docs before code
accepts it.

## Fixture Gate

Before code accepts a new KDF profile, the fixture set must contain at least three
compatibility vectors:

- one old `SKP1` `KDF ID 0x01` vector that still opens;
- one new-profile vector with non-empty plaintext and AAD;
- one new-profile vector with binary plaintext or binary AAD.

The fixture README must name the profile, password, plaintext, AAD, and expected
compatibility behavior for each vector.
