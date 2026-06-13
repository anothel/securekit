#ifndef SECUREKIT_ERROR_HPP_
#define SECUREKIT_ERROR_HPP_

#include <stdexcept>
#include <string>

#include "securekit/export.hpp"

namespace securekit
{

enum class error_code
{
	invalid_input,
	invalid_encoding,
	invalid_packet,
	authentication_failed,
	backend_failure,
};

class SECUREKIT_API error : public std::runtime_error
{
public:
	error(error_code code, std::string message);

	[[nodiscard]] error_code code() const noexcept;

private:
	error_code code_;
};

} // namespace securekit

#endif // SECUREKIT_ERROR_HPP_
