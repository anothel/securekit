#include "securekit/version.hpp"

namespace securekit
{

std::string_view version() noexcept
{
	return SECUREKIT_VERSION;
}

int version_major() noexcept
{
	return SECUREKIT_VERSION_MAJOR;
}

int version_minor() noexcept
{
	return SECUREKIT_VERSION_MINOR;
}

int version_patch() noexcept
{
	return SECUREKIT_VERSION_PATCH;
}

} // namespace securekit
