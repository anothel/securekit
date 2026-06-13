#include "amv/error.hpp"

#include <utility>

namespace amv
{

error::error(error_code code, std::string message) : std::runtime_error(std::move(message)), code_(code)
{
}

error_code error::code() const noexcept
{
	return code_;
}

} // namespace amv
