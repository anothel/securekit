# SecureKit

SecureKit is a small C++20 security utility library, formerly named
Awesome-mix-vol.1.

SecureKit does not implement crypto algorithms. It wraps OpenSSL 3.x behind a small
byte-oriented C++ API.

## What This Project Is

SecureKit is for application code that needs common binary and crypto helpers without
pulling OpenSSL calls through the whole codebase.

The current identity is:

- Practical C++20 utility library.
- OpenSSL-backed crypto, not custom crypto.
- Free-function API first.
- Distinct packet streaming objects where incremental AEAD is useful.
- Additional object-oriented APIs may come later if real call sites need them.

## Features

- Hex encode and decode.
- Base64 encode and decode.
- Base64URL encode and decode.
- SHA-256 digest.
- HMAC-SHA-256 digest.
- HKDF-SHA-256 key derivation.
- Constant-time byte comparison for equal-length secret values.
- Cryptographically secure random bytes.
- URL-safe random token generation.
- AES-256-GCM packet encryption and decryption.
- Move-only packet streaming encryptor and decryptor for `SKT1`.
- AES-256-GCM key wrapping helpers.
- Chunked file sealing and opening with path and stream APIs.
- Password-based chunked file sealing and opening with `SKP1` and scrypt.

## Non-goals

- TLS or networking. Use TLS libraries for that.
- Generic password hashing or general-purpose password KDF APIs.
- Generic streaming object families beyond the `SKT1` packet slice.
- Custom string classes or allocators.
- User-selected algorithms or caller-selected nonces.
- Text encoding or string-to-byte conversion helpers.
- Secure key storage.
- Guaranteed key erasure.
- Homegrown cryptographic primitives.

## Requirements

- C++20 compiler.
- CMake 3.20 or newer.
- OpenSSL 3.x, version 3.0 or newer.

## Build

Generic CMake:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSECUREKIT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

Windows with vcpkg:

```powershell
cmake -S . -B build-vcpkg `
  -DSECUREKIT_BUILD_TESTS=ON `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build build-vcpkg --config Release
ctest --test-dir build-vcpkg --build-config Release --output-on-failure
```

If tests cannot find OpenSSL DLLs on Windows, pass:

```powershell
-DSECUREKIT_OPENSSL_RUNTIME_DIR="path\to\vcpkg\installed\x64-windows\bin"
```

If you are not using a vcpkg toolchain file, point CMake at an OpenSSL prefix:

```powershell
cmake -S . -B build-openssl `
  -DSECUREKIT_BUILD_TESTS=ON `
  -DOPENSSL_ROOT_DIR="path\to\openssl-prefix"
```

## Test Dependency

When `SECUREKIT_BUILD_TESTS=ON`, CMake first looks for an installed `GTest`
package. If none is found, it uses `FetchContent` to get GoogleTest v1.14.0.
The first configure for a fresh fallback build tree may need network access.
After that, CMake reuses the downloaded source under the build tree.

GitHub Actions caches `build/_deps` for jobs that build tests, so successful CI
runs can reuse the GoogleTest checkout when the cache key still matches.

For an offline or cached test build, point CMake at an existing GoogleTest
source checkout:

```sh
cmake -S . -B build \
  -DSECUREKIT_BUILD_TESTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/to/googletest-src
```

For package-only builds that do not need tests, disable tests:

```sh
cmake -S . -B build-install-only -DBUILD_TESTING=OFF -DSECUREKIT_BUILD_TESTS=OFF
```

## Install

```sh
cmake --install build --prefix /path/to/prefix --config Release
```

## CLI

When installed, SecureKit also provides a small `securekit` utility executable:

```sh
securekit --version
securekit version
securekit token 32
securekit sha256 --text abc
securekit sha256 --file path/to/file.bin
securekit hmac-sha256 --key-hex 4a656665 --text "what do ya want for nothing?"
securekit hmac-sha256 --key-hex 4a656665 --file path/to/file.bin
securekit hkdf-sha256 --key-hex 0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b --salt-hex 000102030405060708090a0b0c --info-hex f0f1f2f3f4f5f6f7f8f9 --out-size 42
securekit hkdf-sha256 --key-hex 4a656665 --salt-hex 73616c74 --info-text context --out-size 32
securekit hex-encode --text abc
securekit hex-decode --text 616263
securekit base64url-encode --text abc
securekit base64url-decode --text YWJj
securekit keygen --out key.hex
securekit encrypt --text hello --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit decrypt --packet-hex 534b543101000000000000000000000000a6c22c512240180b643bb7b6d19ae91d51db387693b2f165220613f98728de --key-hex 0000000000000000000000000000000000000000000000000000000000000000 --aad-text record:v1
securekit encrypt --in plain.bin --out plain.bin.skt --key-file key.hex --aad-text record:v1
securekit decrypt --packet-file plain.bin.skt --out plain.bin --key-file key.hex --aad-text record:v1
securekit wrap-key --key-hex 101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f --wrapping-key-hex 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
securekit unwrap-key --packet-hex 534b543101... --wrapping-key-hex 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
securekit wrap-key --key-file data-key.hex --wrapping-key-file wrapping-key.hex --out data-key.skt
securekit unwrap-key --packet-file data-key.skt --wrapping-key-file wrapping-key.hex --out recovered-data-key.hex
securekit help seal-file
securekit seal-file --in plain.bin --out plain.bin.skf --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit open-file --in plain.bin.skf --out plain.bin --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex --aad-text record:v1
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex --aad-text record:v1
securekit seal-file --in plain.bin --out plain.bin.skf --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f --aad-hex 7265636f72643a7631
securekit open-file --in plain.bin.skf --out plain.bin --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f --aad-hex 7265636f72643a7631
securekit seal-file --in - --out - --key-file key.hex < plain.bin > plain.bin.skf
securekit open-file --in - --out - --key-file key.hex < plain.bin.skf > plain.bin
securekit help seal-file-password
securekit seal-file-password --in plain.bin --out plain.bin.skp --password-file password.bin
securekit open-file-password --in plain.bin.skp --out plain.bin --password-file password.bin
securekit seal-file-password --in plain.bin --out plain.bin.skp --password-file password.bin --aad-text record:v1
securekit open-file-password --in plain.bin.skp --out plain.bin --password-file password.bin --aad-text record:v1
```

The packet CLI commands use the `SKT1` packet format, and the file commands use
the `SKF1` or `SKP1` file formats. `seal-file` and `open-file` use `SKF1` with
a caller-provided 32-byte key. `seal-file-password` and `open-file-password` use
`SKP1` with scrypt-derived keys from `--password-file`. `--key-hex` must be a
strict 64-character hex string for a 32-byte key. `keygen` writes a fresh key as
64 lowercase hex characters plus a trailing newline and refuses to overwrite an
existing output file. `--key-file` reads the same text format, trimming leading
and trailing ASCII whitespace before strict validation. `--password-file` reads
raw bytes exactly, without trimming or normalization; empty password files are
rejected.

`encrypt` accepts either `--text` or `--in` plus exactly one key source. Without
`--out`, it writes the resulting `SKT1` packet as lowercase hex. With `--out`,
it writes the binary packet and refuses to overwrite an existing file. `decrypt`
accepts either `--packet-hex` or `--packet-file` plus exactly one key source.
Without `--out`, it writes the recovered plaintext bytes to stdout plus one
trailing newline. With `--out`, it writes the raw plaintext bytes to a file and
refuses to overwrite an existing file.

Packet and file commands can also take optional AAD with either `--aad-text` or
`--aad-hex`; the same AAD bytes must be provided to `decrypt`, `open-file`, or
`open-file-password`, and AAD is authenticated but not stored in the packet or
file. `--aad-text` uses the argument bytes directly. `--aad-hex` strictly
decodes hex to bytes.
Packet and file command options may be supplied in any order, but each command
must provide exactly one input source and one key source or password source.
File commands also require exactly one output destination. For `seal-file`,
`open-file`, `seal-file-password`, and `open-file-password`, `--in -` reads from
stdin and `--out -` writes raw binary output to stdout. At most one AAD option
may be provided. The CLI does not expose password prompts, password text
arguments, or environment-variable key loading.

`hmac-sha256` accepts an arbitrary strict hex key and either text or file input.
`hkdf-sha256` accepts strict hex key material and salt, accepts `info` as either
strict hex or argument bytes, and writes the derived bytes as lowercase hex.
`wrap-key` wraps one 32-byte key with another 32-byte wrapping key. Key inputs
may come from strict hex arguments or key files in the same format produced by
`keygen`. Without `--out`, `wrap-key` writes the resulting `SKT1` packet as
lowercase hex. With `--out`, it writes the binary `SKT1` packet and refuses to
overwrite an existing file. `unwrap-key` accepts a wrapped key packet from
strict hex or a binary packet file. Without `--out`, it writes the recovered
32-byte key as lowercase hex. With `--out`, it writes a `keygen`-compatible key
file and refuses to overwrite an existing file. The key wrapping CLI slice does
not expose AAD.

Successful text-output CLI commands write the result plus one trailing newline
to stdout. File commands with `--out -` write raw binary bytes without adding a
newline. Usage, parse, file, or decoding failures return a non-zero exit code
and write a short message to stderr. Text arguments are treated as raw bytes in
the active process encoding; the CLI does not normalize or transcode Unicode.
Decode commands use the same strict validation as the C++ APIs.

Running `securekit`, `securekit help`, or `securekit --help` prints the top-level
usage text. `securekit --version` and `securekit version` print the package
version compiled from the CMake project version. `securekit help <command>`
prints command-specific usage for the supported commands. Known commands with
the wrong shape print that command's usage to stderr. Packet and file command
conflicts report the specific duplicate, conflicting input or packet source,
conflicting key source, conflicting AAD source, or unsupported option.

## Consume With CMake

From an installed package:

```cmake
find_package(securekit CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE securekit::securekit)
```

From a source checkout:

```cmake
add_subdirectory(path/to/securekit)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE securekit::securekit)
```

## C++ Example

```cpp
#include "securekit/securekit.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

securekit::bytes bytes_from_text(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (unsigned char ch : text)
	{
		out.push_back(static_cast<std::byte>(ch));
	}
	return out;
}

int main()
{
	const securekit::bytes message = bytes_from_text("hello securekit");

	const std::string hex = securekit::hex_encode(message);
	const securekit::bytes from_hex = securekit::hex_decode(hex);

	const std::string b64 = securekit::base64_encode(from_hex);
	const securekit::bytes from_b64 = securekit::base64_decode(b64);

	const securekit::digest256 digest = securekit::sha256(from_b64);
	std::cout << "sha256=" << securekit::hex_encode(digest) << "\n";

	const std::string token = securekit::random_token(32);
	std::cout << "token=" << token << "\n";

	const securekit::key256 key = securekit::generate_key();
	const securekit::bytes aad = bytes_from_text("record:v1");
	const securekit::bytes packet = securekit::encrypt(from_b64, key, aad);
	const securekit::bytes plaintext = securekit::decrypt(packet, key, aad);

	std::cout << "round trip bytes=" << plaintext.size() << "\n";
}
```

`bytes_from_text` is example-side code. SecureKit accepts byte spans and does not
define text encoding conversion helpers in v1.

## Streaming Packet Example

```cpp
securekit::packet_encryptor encryptor(key, aad);
securekit::bytes packet = encryptor.begin();
securekit::bytes chunk = encryptor.update(plaintext);
securekit::bytes tag = encryptor.finalize();
packet.insert(packet.end(), chunk.begin(), chunk.end());
packet.insert(packet.end(), tag.begin(), tag.end());

securekit::packet_decryptor decryptor(key, aad);
std::span<const std::byte> packet_span(packet);
decryptor.begin(packet_span.first(17));
securekit::bytes roundtrip = decryptor.update(packet_span.subspan(17, plaintext.size()));
securekit::bytes trailing = decryptor.finalize(packet_span.last(16));
roundtrip.insert(roundtrip.end(), trailing.begin(), trailing.end());
```

`packet_encryptor::begin()` returns the serialized `SKT1` packet prefix
(`header + nonce`). `packet_decryptor::begin()` expects that same 17-byte
prefix. `packet_decryptor::update()` returns decrypted bytes before tag
verification completes, so callers must treat those bytes as untrusted until
`finalize()` succeeds.

## Public API

```cpp
std::string securekit::hex_encode(std::span<const std::byte> input);
securekit::bytes securekit::hex_decode(std::string_view input);

std::string securekit::base64_encode(std::span<const std::byte> input);
securekit::bytes securekit::base64_decode(std::string_view input);
std::string securekit::base64url_encode(std::span<const std::byte> input);
securekit::bytes securekit::base64url_decode(std::string_view input);

securekit::digest256 securekit::sha256(std::span<const std::byte> input);
securekit::digest256 securekit::hmac_sha256(
	std::span<const std::byte> key,
	std::span<const std::byte> input);
securekit::bytes securekit::hkdf_sha256(
	std::span<const std::byte> key_material,
	std::span<const std::byte> salt,
	std::span<const std::byte> info,
	std::size_t output_size);

bool securekit::constant_time_equal(
	std::span<const std::byte> left,
	std::span<const std::byte> right);

securekit::bytes securekit::random_bytes(std::size_t size);
securekit::key256 securekit::generate_key();
std::string securekit::random_token(std::size_t byte_size);

securekit::bytes securekit::encrypt(
	std::span<const std::byte> plaintext,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

securekit::bytes securekit::decrypt(
	std::span<const std::byte> packet,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

namespace securekit {

class packet_encryptor {
public:
	explicit packet_encryptor(const key256 &key, std::span<const std::byte> aad = {});
	packet_encryptor(packet_encryptor &&) noexcept;
	packet_encryptor &operator=(packet_encryptor &&) noexcept;
	bytes begin();
	bytes update(std::span<const std::byte> plaintext);
	bytes finalize();
};

class packet_decryptor {
public:
	explicit packet_decryptor(const key256 &key, std::span<const std::byte> aad = {});
	packet_decryptor(packet_decryptor &&) noexcept;
	packet_decryptor &operator=(packet_decryptor &&) noexcept;
	void begin(std::span<const std::byte> packet_prefix);
	bytes update(std::span<const std::byte> ciphertext);
	bytes finalize(std::span<const std::byte> tag);
};

} // namespace securekit

securekit::bytes securekit::wrap_key(
	const securekit::key256 &key_to_wrap,
	const securekit::key256 &wrapping_key,
	std::span<const std::byte> aad = {});

securekit::key256 securekit::unwrap_key(
	std::span<const std::byte> packet,
	const securekit::key256 &wrapping_key,
	std::span<const std::byte> aad = {});

void securekit::seal_file(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

void securekit::seal_file(
	std::istream &input,
	std::ostream &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

void securekit::open_file(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

void securekit::open_file(
	std::istream &input,
	std::ostream &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

void securekit::seal_file_with_password(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	std::span<const std::byte> password,
	std::span<const std::byte> aad = {});

void securekit::seal_file_with_password(
	std::istream &input,
	std::ostream &output,
	std::span<const std::byte> password,
	std::span<const std::byte> aad = {});

void securekit::open_file_with_password(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	std::span<const std::byte> password,
	std::span<const std::byte> aad = {});

void securekit::open_file_with_password(
	std::istream &input,
	std::ostream &output,
	std::span<const std::byte> password,
	std::span<const std::byte> aad = {});
```

`securekit::error` reports library failures with `securekit::error_code`.

The base API stays free-function oriented. Hex and Base64 decoders are strict
only: malformed or non-canonical input raises `securekit::error`. If permissive
decoding or non-throwing result APIs are needed later, they should be added as
explicitly named variants instead of changing these functions.

The packet API intentionally keeps the general `encrypt` and `decrypt` names.
Incremental packet processing uses distinct `packet_encryptor` and
`packet_decryptor` names instead of overloading those free functions. Both
streaming classes are move-only and one-shot.

`securekit::wrap_key` uses the same `SKT1` packet format as `encrypt` to wrap a
single 32-byte `securekit::key256` with another 32-byte wrapping key. It accepts
optional AAD. `securekit::unwrap_key` authenticates and decrypts the packet, then
rejects packets that do not contain exactly one 32-byte key.

`securekit::random_token` returns an unpadded Base64URL string from
cryptographically secure random bytes. It rejects `byte_size == 0` with
`securekit::error_code::invalid_input`.

## AES-256-GCM Packet Format

SecureKit AES-GCM encryption returns one serialized packet:

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKT1` |
| 4 | 1 | Version | `0x01` |
| 5 | 12 | Nonce | Generated per packet |
| 17 | N | Ciphertext | AES-256-GCM ciphertext |
| 17 + N | 16 | Tag | AES-GCM authentication tag |

The caller supplies a 32-byte key. SecureKit generates the packet nonce internally.
The packet header and caller-provided AAD are authenticated. AAD is not stored
in the packet. Decryption requires the same AAD used during encryption.

## Packet Streaming

`packet_encryptor` and `packet_decryptor` use the same `SKT1` wire format as the
free functions, but split it into:

- `begin()` / `begin(packet_prefix)` for the 17-byte packet prefix.
- `update()` for ciphertext or plaintext chunks.
- `finalize()` for the 16-byte authentication tag.

The classes are move-only and one-shot. Construction fixes the key and AAD for
the whole packet. Invalid sequencing raises
`securekit::error_code::invalid_input`. Malformed prefixes or malformed tag
sizes raise `securekit::error_code::invalid_packet`.

`packet_decryptor::update()` yields plaintext before authentication is complete.
Callers must not release, persist, or trust decrypted bytes until
`finalize(tag)` returns successfully.

## SKF1 File Format

`securekit::seal_file` writes an `SKF1` file. It uses a fixed 1 MiB chunk size,
a random per-file salt, HKDF-SHA256 to derive a per-file key, and AES-256-GCM
for each chunk. The chunk nonce is an 8-byte random file nonce prefix plus a
32-bit big-endian chunk index.

File header:

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKF1` |
| 4 | 1 | Version | `0x01` |
| 5 | 1 | Algorithm | `0x01` for AES-256-GCM with HKDF-SHA256 |
| 6 | 4 | Chunk size | `0x00100000` big-endian, 1 MiB |
| 10 | 32 | Salt | Random per file |
| 42 | 8 | Nonce prefix | Random per file |

Each chunk record:

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Chunk index | Big-endian, starts at 0 |
| 4 | 4 | Plaintext size | Big-endian, 0 to 1 MiB |
| 8 | 1 | Final flag | `0x00` non-final, `0x01` final |
| 9 | N | Ciphertext | AES-256-GCM ciphertext |
| 9 + N | 16 | Tag | AES-GCM authentication tag |

The file header, chunk index, plaintext size, final flag, and caller-provided
AAD are authenticated with every chunk. `open_file` rejects malformed headers,
truncated records, appended data, reordered chunks, wrong keys, wrong AAD, and
tag failures. Path overload output files must not already exist; SecureKit
writes a temporary file in the output directory and renames it only after
successful completion. Stream overloads operate on caller-provided streams and
do not perform output path checks or temporary-file renames. When opening from a
stream, callers should treat plaintext output as accepted only after the
function returns successfully.

## SKP1 Password File Format

`securekit::seal_file_with_password` writes an `SKP1` file. It uses the same
1 MiB chunk record format and AES-256-GCM chunk encryption as `SKF1`, but
derives the 32-byte file key from caller-provided password bytes using OpenSSL
scrypt. Password bytes are accepted exactly as supplied by the caller; SecureKit
does not trim, normalize, encode, or prompt for passwords. Empty passwords are
rejected with `securekit::error_code::invalid_input`.

Fixed scrypt parameters:

- `N = 32768`
- `r = 8`
- `p = 1`
- `maxmem = 64 MiB`

File header:

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `SKP1` |
| 4 | 1 | Version | `0x01` |
| 5 | 1 | Cipher | `0x01` for AES-256-GCM |
| 6 | 1 | KDF | `0x01` for scrypt |
| 7 | 1 | Flags | `0x00` |
| 8 | 4 | Chunk size | `0x00100000` big-endian, 1 MiB |
| 12 | 32 | Salt | Random per file |
| 44 | 8 | Nonce prefix | Random per file |
| 52 | 4 | scrypt N | `32768` big-endian |
| 56 | 4 | scrypt r | `8` big-endian |
| 60 | 4 | scrypt p | `1` big-endian |

The `SKP1` header, chunk index, plaintext size, final flag, and caller-provided
AAD are authenticated with every chunk. `open_file_with_password` rejects
malformed headers, unsupported scrypt parameters, truncated records, appended
data, reordered chunks, wrong passwords, wrong AAD, and tag failures. Wrong
passwords and tag failures report the same generic
`securekit::error_code::authentication_failed` message as other file
authentication failures.

## Security Boundaries

SecureKit is a thin C++ API over OpenSSL 3.x. It does not:

- Store keys.
- Provide a general-purpose password hashing API.
- Provide a reusable password KDF API outside `SKP1` file encryption.
- Prevent key copies.
- Guarantee memory erasure.
- Configure OpenSSL providers.
- Hide plaintext from process memory.

SecureKit does not scrub, lock, or otherwise guarantee erasure of key material,
derived keys, plaintext, or intermediate buffers from process memory.

Applications remain responsible for key lifecycle, provider configuration,
process isolation, persistence, backups, logging policy, and threat modeling.

`securekit::constant_time_equal` avoids content-dependent early exits for the
bytes it compares. It returns false for different lengths, but input lengths are
not hidden.

## Password KDF Scope

SecureKit's password support is limited to `SKP1` file encryption. It does not
expose a reusable password KDF API or a password hashing API. `SKP1` currently
uses OpenSSL scrypt with fixed parameters recorded in the file header and
rejects headers that request different parameters.

Future password formats should be versioned as new file formats or explicit
format revisions instead of silently changing `SKP1` behavior. Argon2id,
PBKDF2, prompt handling, environment-variable password loading, password text
arguments, and tunable scrypt parameters are intentionally outside this API
slice.

## OpenSSL Providers and Backend Errors

SecureKit uses OpenSSL's default library context and the provider configuration
already active in the process. It does not load providers, create an
`OSSL_LIB_CTX`, set property queries, or switch between default, legacy, and FIPS
providers.

Applications that require FIPS mode or custom provider selection must configure
OpenSSL before calling SecureKit. AES-256-GCM, SHA-256, HMAC-SHA-256,
HKDF-SHA-256, scrypt, and OpenSSL's random byte APIs must be available from
that configuration.

OpenSSL allocation, initialization, cipher, digest, MAC, KDF, or
random-generation failures are reported as
`securekit::error_code::backend_failure`. SecureKit does not add OpenSSL
error-queue details to public exception messages. AEAD packet, SKF1 chunk, and
SKP1 chunk tag verification failures are reported separately as
`securekit::error_code::authentication_failed` with generic messages.

## Compatibility Fixtures

Known v1 wire-format vectors live in `tests/fixtures` as lowercase hex files.
The test suite reads these files for `SKT1`, `SKF1`, `SKP1`, and key wrapping
coverage instead of keeping serialized packets inline in test source.

## Continuous Integration

GitHub Actions builds and tests:

- Ubuntu GCC Release.
- Ubuntu GCC Debug.
- Ubuntu Clang Release.
- Windows MSVC Release with vcpkg OpenSSL.
- Install and consumer-project checks.
- Install-only package and consumer check with tests disabled.
- Linux static-library package and consumer check.
- Windows shared-library package and consumer check.

Run result is only available after pushing a commit or manually starting the
workflow in GitHub. Local equivalent:

```sh
cmake -S . -B build -DSECUREKIT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
cmake --install build --config Release --prefix ./install
cmake -DSECUREKIT_CLI=./install/bin/securekit -DSECUREKIT_CLI_WORK_DIR=./installed-cli-check -P tests/package/check_installed_cli.cmake
cmake -S tests/consumer -B consumer-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=./install
cmake --build consumer-build --config Release
./consumer-build/securekit_consumer
```

Install-only package check:

```sh
cmake -S . -B build-install-only -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSECUREKIT_BUILD_TESTS=OFF
cmake --build build-install-only --config Release
cmake --install build-install-only --config Release --prefix ./install-only
test -f ./install-only/bin/securekit
test -f ./install-only/lib/cmake/securekit/securekitConfig.cmake
test -f ./install-only/lib/cmake/securekit/securekitConfigVersion.cmake
test -f ./install-only/lib/cmake/securekit/securekitTargets.cmake
test -f ./install-only/lib/cmake/securekit/securekitTargets-release.cmake
cmake -DSECUREKIT_CLI=./install-only/bin/securekit -DSECUREKIT_CLI_WORK_DIR=./install-only-cli-check -P tests/package/check_installed_cli.cmake
cmake -S tests/consumer -B consumer-install-only-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=./install-only
cmake --build consumer-install-only-build --config Release
./consumer-install-only-build/securekit_consumer
```

Linux static-library package check:

```sh
cmake -S . -B build-static -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DSECUREKIT_BUILD_TESTS=ON
cmake --build build-static --config Release
ctest --test-dir build-static --build-config Release --output-on-failure
cmake --install build-static --config Release --prefix ./install-static
test -f ./install-static/bin/securekit
test -f ./install-static/lib/libsecurekit.a
test ! -f ./install-static/lib/libsecurekit.so
test -f ./install-static/lib/cmake/securekit/securekitConfig.cmake
test -f ./install-static/lib/cmake/securekit/securekitConfigVersion.cmake
test -f ./install-static/lib/cmake/securekit/securekitTargets.cmake
test -f ./install-static/lib/cmake/securekit/securekitTargets-release.cmake
cmake -DSECUREKIT_CLI=./install-static/bin/securekit -DSECUREKIT_CLI_WORK_DIR=./install-static-cli-check -P tests/package/check_installed_cli.cmake
cmake -S tests/consumer -B consumer-static-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=./install-static
cmake --build consumer-static-build --config Release
./consumer-static-build/securekit_consumer
```

Windows shared-library package check:

```powershell
cmake -S . -B build-vcpkg-shared `
  -DBUILD_SHARED_LIBS=ON `
  -DSECUREKIT_BUILD_TESTS=ON `
  -DOPENSSL_ROOT_DIR="path\to\openssl-prefix" `
  -DSECUREKIT_OPENSSL_RUNTIME_DIR="path\to\openssl-bin"

cmake --build build-vcpkg-shared --config Release
ctest --test-dir build-vcpkg-shared --build-config Release --output-on-failure
cmake --install build-vcpkg-shared --config Release --prefix .\install-shared
Test-Path .\install-shared\bin\securekit.exe
$env:PATH = ".\install-shared\bin;path\to\openssl-bin;$env:PATH"
cmake -DSECUREKIT_CLI=.\install-shared\bin\securekit.exe -DSECUREKIT_CLI_WORK_DIR=.\install-shared-cli-check -P tests\package\check_installed_cli.cmake
cmake -S tests\consumer -B consumer-shared-build -DCMAKE_PREFIX_PATH=.\install-shared -DOPENSSL_ROOT_DIR="path\to\openssl-prefix"
cmake --build consumer-shared-build --config Release
.\consumer-shared-build\Release\securekit_consumer.exe
```

On Windows with dynamically linked OpenSSL, put the OpenSSL DLL directory on
`PATH` before running the consumer executable:

```powershell
$env:PATH = "path\to\openssl-bin;$env:PATH"
.\consumer-build\Release\securekit_consumer.exe
```

## Roadmap

Near-term:

- Keep free-function API stable.
- Add more test vectors where useful.
- Keep package consumption simple.

Later:

- Object-oriented APIs if repeated call sites justify them.
- Additional password formats only if parameter agility or dependency changes justify them.
- Additional streaming APIs if repeated call sites need other incremental formats.
