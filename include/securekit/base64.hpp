#ifndef SECUREKIT_BASE64_HPP_
#define SECUREKIT_BASE64_HPP_

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API std::string base64_encode(std::span<const std::byte> input);
SECUREKIT_API bytes base64_decode(std::string_view input);
SECUREKIT_API std::string base64url_encode(std::span<const std::byte> input);
SECUREKIT_API bytes base64url_decode(std::string_view input);

} // namespace securekit

#endif // SECUREKIT_BASE64_HPP_
