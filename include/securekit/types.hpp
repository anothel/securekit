#ifndef SECUREKIT_TYPES_HPP_
#define SECUREKIT_TYPES_HPP_

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace securekit
{

using bytes = std::vector<std::byte>;
using key256 = std::array<std::byte, 32>;
using digest256 = std::array<std::byte, 32>;

} // namespace securekit

#endif // SECUREKIT_TYPES_HPP_
