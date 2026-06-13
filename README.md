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
- SHA-256 digest.
- Cryptographically secure random bytes.
- AES-256-GCM packet encryption and decryption.

## Non-goals

- TLS or networking. Use TLS libraries for that.
- Password-based encryption.
- File or streaming encryption in v1.
- Custom string classes or allocators.
- User-selected algorithms or caller-selected nonces.
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

	const securekit::key256 key = securekit::generate_key();
	const securekit::bytes aad = bytes_from_text("record:v1");
	const securekit::bytes packet = securekit::encrypt(from_b64, key, aad);
	const securekit::bytes plaintext = securekit::decrypt(packet, key, aad);

	std::cout << "round trip bytes=" << plaintext.size() << "\n";
}
```

## Public API

```cpp
std::string securekit::hex_encode(std::span<const std::byte> input);
securekit::bytes securekit::hex_decode(std::string_view input);

std::string securekit::base64_encode(std::span<const std::byte> input);
securekit::bytes securekit::base64_decode(std::string_view input);

securekit::digest256 securekit::sha256(std::span<const std::byte> input);

securekit::bytes securekit::random_bytes(std::size_t size);
securekit::key256 securekit::generate_key();

securekit::bytes securekit::encrypt(
	std::span<const std::byte> plaintext,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});

securekit::bytes securekit::decrypt(
	std::span<const std::byte> packet,
	const securekit::key256 &key,
	std::span<const std::byte> aad = {});
```

`securekit::error` reports library failures with `securekit::error_code`.

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

## Security Boundaries

SecureKit is a thin C++ API over OpenSSL 3.x. It does not:

- Store keys.
- Derive keys from passwords.
- Prevent key copies.
- Guarantee memory erasure.
- Configure OpenSSL providers.
- Hide plaintext from process memory.

Applications remain responsible for key lifecycle, provider configuration,
process isolation, persistence, backups, logging policy, and threat modeling.

## Continuous Integration

GitHub Actions builds and tests:

- Ubuntu GCC Release.
- Ubuntu GCC Debug.
- Ubuntu Clang Release.
- Windows MSVC Release with vcpkg OpenSSL.
- Install and consumer-project checks.
- Install-only package check with tests disabled.
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
- Streaming/file encryption only with clear packet format decisions.
