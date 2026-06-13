#ifndef AMV_RANDOM_HPP_
#define AMV_RANDOM_HPP_

#include <cstddef>

#include "amv/export.hpp"
#include "amv/types.hpp"

namespace amv
{

AMV_API bytes random_bytes(std::size_t size);
AMV_API key256 generate_key();

} // namespace amv

#endif // AMV_RANDOM_HPP_
