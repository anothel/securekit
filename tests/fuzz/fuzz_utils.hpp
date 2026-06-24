#ifndef SECUREKIT_TESTS_FUZZ_FUZZ_UTILS_HPP_
#define SECUREKIT_TESTS_FUZZ_FUZZ_UTILS_HPP_

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "securekit/error.hpp"
#include "securekit/hex.hpp"
#include "securekit/types.hpp"

namespace securekit::fuzz
{

inline bytes bytes_from_data(const std::uint8_t *data, std::size_t size)
{
	bytes out(size);
	for (std::size_t index = 0; index < size; ++index)
	{
		out[index] = static_cast<std::byte>(data[index]);
	}
	return out;
}

inline std::string string_from_data(const std::uint8_t *data, std::size_t size)
{
	return std::string(reinterpret_cast<const char *>(data), size);
}

inline bool is_ascii_space(char ch)
{
	return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\f' || ch == '\v';
}

inline bool is_lower_hex(char ch)
{
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
}

inline bool canonical_hex_fixture(std::string_view input)
{
	bool saw_digit = false;
	std::size_t digits = 0;
	for (const char ch : input)
	{
		if (is_ascii_space(ch))
		{
			continue;
		}
		if (!is_lower_hex(ch))
		{
			return false;
		}
		saw_digit = true;
		++digits;
	}
	return saw_digit && (digits % 2) == 0;
}

inline bytes raw_or_decoded_fixture(const std::uint8_t *data, std::size_t size)
{
	const std::string input = string_from_data(data, size);
	if (canonical_hex_fixture(input))
	{
		return hex_decode(input);
	}
	return bytes_from_data(data, size);
}

inline key256 key_from_seed(unsigned int seed)
{
	key256 key{};
	for (std::size_t index = 0; index < key.size(); ++index)
	{
		key[index] = static_cast<std::byte>((seed + index) & 0xffu);
	}
	return key;
}

} // namespace securekit::fuzz

#endif // SECUREKIT_TESTS_FUZZ_FUZZ_UTILS_HPP_
