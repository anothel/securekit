#ifndef SECUREKIT_AEAD_HPP_
#define SECUREKIT_AEAD_HPP_

#include <cstddef>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API bytes encrypt(std::span<const std::byte> plaintext, const key256 &key, std::span<const std::byte> aad = {});
SECUREKIT_API bytes decrypt(std::span<const std::byte> packet, const key256 &key, std::span<const std::byte> aad = {});

} // namespace securekit

#endif // SECUREKIT_AEAD_HPP_
