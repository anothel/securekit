#ifndef SECUREKIT_KEY_WRAP_HPP_
#define SECUREKIT_KEY_WRAP_HPP_

#include <cstddef>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API bytes wrap_key(const key256 &key_to_wrap, const key256 &wrapping_key, std::span<const std::byte> aad = {});

SECUREKIT_API key256 unwrap_key(std::span<const std::byte> packet, const key256 &wrapping_key, std::span<const std::byte> aad = {});

} // namespace securekit

#endif // SECUREKIT_KEY_WRAP_HPP_
