#ifndef SECUREKIT_RANDOM_HPP_
#define SECUREKIT_RANDOM_HPP_

#include <cstddef>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API bytes random_bytes(std::size_t size);
SECUREKIT_API key256 generate_key();

} // namespace securekit

#endif // SECUREKIT_RANDOM_HPP_
