#ifndef SECUREKIT_VERSION_HPP_
#define SECUREKIT_VERSION_HPP_

#include <string_view>

#include "securekit/export.hpp"

namespace securekit
{

SECUREKIT_API std::string_view version() noexcept;
SECUREKIT_API int version_major() noexcept;
SECUREKIT_API int version_minor() noexcept;
SECUREKIT_API int version_patch() noexcept;

} // namespace securekit

#endif // SECUREKIT_VERSION_HPP_
