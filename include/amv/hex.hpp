#ifndef AMV_HEX_HPP_
#define AMV_HEX_HPP_

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "amv/export.hpp"
#include "amv/types.hpp"

namespace amv
{

AMV_API std::string hex_encode(std::span<const std::byte> input);
AMV_API bytes hex_decode(std::string_view input);

} // namespace amv

#endif // AMV_HEX_HPP_
