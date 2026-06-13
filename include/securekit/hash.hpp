#ifndef SECUREKIT_HASH_HPP_
#define SECUREKIT_HASH_HPP_

#include <cstddef>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API digest256 sha256(std::span<const std::byte> input);

} // namespace securekit

#endif // SECUREKIT_HASH_HPP_
