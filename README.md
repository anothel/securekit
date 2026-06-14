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
- Object-oriented APIs may come later if real call sites need them.

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
- Chunked file sealing and opening.

## Non-goals

- TLS or networking. Use TLS libraries for that.
- Password-based encryption.
- Streaming object APIs in v1.
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

void securekit::seal_file(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

void securekit::open_file(
	const std::filesystem::path &input,
	const std::filesystem::path &output,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});
```

`securekit::error` reports library failures with `securekit::error_code`.

The v1 API stays free-function oriented. Hex and Base64 decoders are strict
only: malformed or non-canonical input raises `securekit::error`. If permissive
decoding or non-throwing result APIs are needed later, they should be added as
explicitly named variants instead of changing these functions.

The v1 AEAD API intentionally keeps the general `encrypt` and `decrypt` names.
Future file, streaming, or key-wrapping APIs should use distinct names instead
of overloading these packet functions.

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
tag failures. Output files must not already exist; SecureKit writes a temporary
file in the output directory and renames it only after successful completion.

## Security Boundaries

SecureKit is a thin C++ API over OpenSSL 3.x. It does not:

- Store keys.
- Derive keys from passwords.
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

## Password KDF Direction

SecureKit does not provide password-based encryption or password hashing APIs in
v1. When those APIs are designed, the preferred password KDF should be Argon2id
because it is memory-hard and is the current general-purpose recommendation for
new password storage designs.

If Argon2id is not available, scrypt is the next non-FIPS fallback. If FIPS 140
compliance or an OpenSSL-only dependency policy is required, use
PBKDF2-HMAC-SHA256 with a high work factor instead. PBKDF2 should not be the
default for new non-FIPS password APIs because it is CPU-hard, not memory-hard.

SecureKit should not expose a password KDF API until the parameter encoding,
salt generation, upgrade strategy, provider/dependency story, and denial-of-service
limits are designed together. Work factors must be benchmarked on the target
deployment rather than treated as permanent constants.

## OpenSSL Providers and Backend Errors

SecureKit uses OpenSSL's default library context and the provider configuration
already active in the process. It does not load providers, create an
`OSSL_LIB_CTX`, set property queries, or switch between default, legacy, and FIPS
providers.

Applications that require FIPS mode or custom provider selection must configure
OpenSSL before calling SecureKit. AES-256-GCM, SHA-256, HMAC-SHA-256,
HKDF-SHA-256, and OpenSSL's random byte APIs must be available from that
configuration.

OpenSSL allocation, initialization, cipher, digest, MAC, KDF, or
random-generation failures are reported as
`securekit::error_code::backend_failure`. SecureKit does not add OpenSSL
error-queue details to public exception messages. AEAD packet and SKF1 chunk tag
verification failures are reported separately as
`securekit::error_code::authentication_failed` with generic messages.

## Continuous Integration

GitHub Actions builds and tests:

- Ubuntu GCC Release.
- Ubuntu GCC Debug.
- Ubuntu Clang Release.
- Windows MSVC Release with vcpkg OpenSSL.
- Install and consumer-project checks.
- Install-only package check with tests disabled.
- Linux static-library package and consumer check.
- Windows shared-library package and consumer check.

Run result is only available after pushing a commit or manually starting the
workflow in GitHub. Local equivalent:

```sh
cmake -S . -B build -DSECUREKIT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
cmake --install build --config Release --prefix ./install
cmake -S tests/consumer -B consumer-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=./install
cmake --build consumer-build --config Release
./consumer-build/securekit_consumer
```

Install-only package check:

```sh
cmake -S . -B build-install-only -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSECUREKIT_BUILD_TESTS=OFF
cmake --build build-install-only --config Release
cmake --install build-install-only --config Release --prefix ./install-only
test -f ./install-only/lib/cmake/securekit/securekitConfig.cmake
test -f ./install-only/lib/cmake/securekit/securekitConfigVersion.cmake
test -f ./install-only/lib/cmake/securekit/securekitTargets.cmake
test -f ./install-only/lib/cmake/securekit/securekitTargets-release.cmake
```

Linux static-library package check:

```sh
cmake -S . -B build-static -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DSECUREKIT_BUILD_TESTS=ON
cmake --build build-static --config Release
ctest --test-dir build-static --build-config Release --output-on-failure
cmake --install build-static --config Release --prefix ./install-static
test -f ./install-static/lib/libsecurekit.a
test ! -f ./install-static/lib/libsecurekit.so
test -f ./install-static/lib/cmake/securekit/securekitConfig.cmake
test -f ./install-static/lib/cmake/securekit/securekitConfigVersion.cmake
test -f ./install-static/lib/cmake/securekit/securekitTargets.cmake
test -f ./install-static/lib/cmake/securekit/securekitTargets-release.cmake
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
cmake -S tests\consumer -B consumer-shared-build -DCMAKE_PREFIX_PATH=.\install-shared -DOPENSSL_ROOT_DIR="path\to\openssl-prefix"
cmake --build consumer-shared-build --config Release
$env:PATH = ".\install-shared\bin;path\to\openssl-bin;$env:PATH"
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
- Password-based encryption with a deliberate KDF design.
- Streaming APIs if repeated call sites need incremental processing.
