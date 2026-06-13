#ifndef SECUREKIT_RANDOM_HPP_
#define SECUREKIT_RANDOM_HPP_

#include <cstddef>
#include <string>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API bytes random_bytes(std::size_t size);
SECUREKIT_API key256 generate_key();
SECUREKIT_API std::string random_token(std::size_t byte_size);

} // namespace securekit

#endif // SECUREKIT_RANDOM_HPP_
