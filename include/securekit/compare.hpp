#ifndef SECUREKIT_COMPARE_HPP_
#define SECUREKIT_COMPARE_HPP_

#include <cstddef>
#include <span>

#include "securekit/export.hpp"

namespace securekit
{

// Avoids content-dependent early exits for compared bytes. Input lengths are
// checked first and must not be secret.
SECUREKIT_API bool constant_time_equal(std::span<const std::byte> left, std::span<const std::byte> right);

} // namespace securekit

#endif // SECUREKIT_COMPARE_HPP_
