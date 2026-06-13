#ifndef AMV_BASE64_HPP_
#define AMV_BASE64_HPP_

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "amv/export.hpp"
#include "amv/types.hpp"

namespace amv
{

AMV_API std::string base64_encode(std::span<const std::byte> input);
AMV_API bytes base64_decode(std::string_view input);

} // namespace amv

#endif // AMV_BASE64_HPP_
