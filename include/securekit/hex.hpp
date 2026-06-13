#ifndef SECUREKIT_HEX_HPP_
#define SECUREKIT_HEX_HPP_

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API std::string hex_encode(std::span<const std::byte> input);
SECUREKIT_API bytes hex_decode(std::string_view input);

} // namespace securekit

#endif // SECUREKIT_HEX_HPP_
