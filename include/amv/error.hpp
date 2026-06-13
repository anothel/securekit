#ifndef AMV_ERROR_HPP_
#define AMV_ERROR_HPP_

#include <stdexcept>
#include <string>

#include "amv/export.hpp"

namespace amv
{

enum class error_code
{
	invalid_input,
	invalid_encoding,
	invalid_packet,
	authentication_failed,
	backend_failure,
};

class AMV_API error : public std::runtime_error
{
public:
	error(error_code code, std::string message);

	[[nodiscard]] error_code code() const noexcept;

private:
	error_code code_;
};

} // namespace amv

#endif // AMV_ERROR_HPP_
