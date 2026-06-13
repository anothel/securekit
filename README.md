# Awesome-mix-vol.1

Awesome Modules, Volume 1 (AMV) is a small C++20 library for byte-oriented
crypto utilities.

AMV does not implement crypto algorithms. It uses OpenSSL 3.x through a small
C++ API.

## Features

- Hex encode and decode.
- Base64 encode and decode.
- SHA-256.
- Cryptographically secure random bytes.
- AES-256-GCM packet encryption.

## Non-goals

- TLS or networking.
- Password-based encryption.
- File or streaming encryption in v1.
- Custom string classes or allocators.
- User-selected algorithms or nonces.
- Secure key storage or guaranteed key erasure.

## Requirements

- C++20 compiler.
- CMake 3.20 or newer.
- OpenSSL 3.x, version 3.0 or newer within the 3.x major release.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DAMV_BUILD_TESTS=OFF
cmake --build build --config Release
```

## Install

```sh
cmake --install build --prefix /path/to/prefix --config Release
```

## Consume With CMake

From an installed package:

```cmake
find_package(amv CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE amv::amv)
```

From a source checkout:

```cmake
add_subdirectory(path/to/Awesome-mix-vol.1)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE amv::amv)
```

## C++ Example

```cpp
#include "amv/amv.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

amv::bytes bytes_from_text(std::string_view text) {
  amv::bytes out;
  out.reserve(text.size());
  for (unsigned char ch : text) {
    out.push_back(static_cast<std::byte>(ch));
  }
  return out;
}

int main() {
  const amv::bytes message = bytes_from_text("hello amv");

  const std::string hex = amv::hex_encode(message);
  const amv::bytes from_hex = amv::hex_decode(hex);

  const std::string b64 = amv::base64_encode(from_hex);
  const amv::bytes from_b64 = amv::base64_decode(b64);

  const amv::digest256 digest = amv::sha256(from_b64);
  std::cout << "sha256=" << amv::hex_encode(digest) << "\n";

  const amv::key256 key = amv::generate_key();
  const amv::bytes aad = bytes_from_text("record:v1");
  const amv::bytes packet = amv::encrypt(from_b64, key, aad);
  const amv::bytes plaintext = amv::decrypt(packet, key, aad);

  std::cout << "round trip bytes=" << plaintext.size() << "\n";
}
```

## AES-256-GCM Packet Format

AMV AES-GCM encryption returns one serialized packet:

| Offset | Size | Field | Value |
| --- | ---: | --- | --- |
| 0 | 4 | Magic | `AMV1` |
| 4 | 1 | Version | `0x01` |
| 5 | 12 | Nonce | Generated per packet |
| 17 | N | Ciphertext | AES-256-GCM ciphertext |
| 17 + N | 16 | Tag | AES-GCM authentication tag |

The caller supplies a 32-byte key. AMV generates the packet nonce internally.
The packet header and caller-provided AAD are authenticated. AAD is not stored
in the packet.

## Security Boundaries

AMV is a thin C++ API over OpenSSL 3.x. It does not:

- Store keys.
- Derive keys from passwords.
- Prevent key copies.
- Guarantee memory erasure.
- Configure OpenSSL providers.

Applications remain responsible for key lifecycle, provider configuration,
process isolation, persistence, backups, logging policy, and threat modeling.

## Roadmap

The v1 API starts with stable free functions for byte-oriented utilities.
Future versions may add object-oriented streaming APIs.
