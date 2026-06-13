#ifndef SECUREKIT_HASH_HPP_
#define SECUREKIT_HASH_HPP_

#include <cstddef>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API digest256 sha256(std::span<const std::byte> input);
SECUREKIT_API digest256 hmac_sha256(std::span<const std::byte> key, std::span<const std::byte> input);
SECUREKIT_API bytes hkdf_sha256(
    std::span<const std::byte> key_material,
    std::span<const std::byte> salt,
    std::span<const std::byte> info,
    std::size_t output_size);

} // namespace securekit

#endif // SECUREKIT_HASH_HPP_
