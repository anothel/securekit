#include "securekit/hex.hpp"

#include "securekit/error.hpp"

namespace
{

char encode_nibble(unsigned int value)
{
	constexpr char kDigits[] = "0123456789abcdef";
	return kDigits[value & 0x0fU];
}

int decode_nibble(char value)
{
	if (value >= '0' && value <= '9')
	{
		return value - '0';
	}
	if (value >= 'a' && value <= 'f')
	{
		return value - 'a' + 10;
	}
	if (value >= 'A' && value <= 'F')
	{
		return value - 'A' + 10;
	}
	return -1;
}

} // namespace

namespace securekit
{

std::string hex_encode(std::span<const std::byte> input)
{
	std::string result;
	if (input.size() > result.max_size() / 2)
	{
		throw error(error_code::invalid_input, "hex input is too large to encode");
	}
	result.reserve(input.size() * 2);

	for (std::byte byte : input)
	{
		const auto value = static_cast<unsigned int>(byte);
		result.push_back(encode_nibble(value >> 4));
		result.push_back(encode_nibble(value));
	}

	return result;
}

bytes hex_decode(std::string_view input)
{
	if ((input.size() % 2) != 0)
	{
		throw error(error_code::invalid_encoding, "hex input must contain an even number of characters");
	}

	bytes result;
	result.reserve(input.size() / 2);

	for (std::size_t index = 0; index < input.size(); index += 2)
	{
		const int high = decode_nibble(input[index]);
		const int low = decode_nibble(input[index + 1]);
		if (high < 0 || low < 0)
		{
			throw error(error_code::invalid_encoding, "hex input contains a non-hexadecimal character");
		}
		result.push_back(static_cast<std::byte>((high << 4) | low));
	}

	return result;
}

} // namespace securekit
