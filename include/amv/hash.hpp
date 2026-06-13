#ifndef AMV_HASH_HPP_
#define AMV_HASH_HPP_

#include <cstddef>
#include <span>

#include "amv/export.hpp"
#include "amv/types.hpp"

namespace amv
{

AMV_API digest256 sha256(std::span<const std::byte> input);

} // namespace amv

#endif // AMV_HASH_HPP_
