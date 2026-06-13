#ifndef AMV_TYPES_HPP_
#define AMV_TYPES_HPP_

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace amv
{

using bytes = std::vector<std::byte>;
using key256 = std::array<std::byte, 32>;
using digest256 = std::array<std::byte, 32>;

} // namespace amv

#endif // AMV_TYPES_HPP_
