#include "amv/base64.hpp"

#include "amv/error.hpp"

namespace
{

constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int decode_char(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
	{
		return ch - 'A';
	}
	if (ch >= 'a' && ch <= 'z')
	{
		return ch - 'a' + 26;
	}
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0' + 52;
	}
	if (ch == '+')
	{
		return 62;
	}
	if (ch == '/')
	{
		return 63;
	}
	return -1;
}

void reject_invalid_base64(std::string_view input)
{
	if ((input.size() % 4) != 0)
	{
		throw amv::error(amv::error_code::invalid_encoding, "base64 input length must be a multiple of four");
	}

	bool seen_padding = false;
	std::size_t padding = 0;

	for (std::size_t i = 0; i < input.size(); ++i)
	{
		const char ch = input[i];
		if (ch == '=')
		{
			seen_padding = true;
			++padding;
			if (padding > 2)
			{
				throw amv::error(amv::error_code::invalid_encoding, "base64 input has too much padding");
			}
			continue;
		}

		if (seen_padding)
		{
			throw amv::error(amv::error_code::invalid_encoding, "base64 input has data after padding");
		}

		if (decode_char(ch) < 0)
		{
			throw amv::error(amv::error_code::invalid_encoding, "base64 input contains an invalid character");
		}
	}

	if (padding > 0 && input.size() < 4)
	{
		throw amv::error(amv::error_code::invalid_encoding, "base64 padding without data");
	}
}

} // namespace

namespace amv
{

std::string base64_encode(std::span<const std::byte> input)
{
	std::string output;
	const std::size_t trailing_group = input.size() % 3 == 0 ? 0U : 1U;
	const std::size_t encoded_groups = input.size() / 3 + trailing_group;
	if (encoded_groups > output.max_size() / 4)
	{
		throw error(error_code::invalid_input, "base64 input is too large to encode");
	}
	output.reserve(encoded_groups * 4);

	for (std::size_t i = 0; i < input.size(); i += 3)
	{
		const bool has_b1 = i + 1 < input.size();
		const bool has_b2 = i + 2 < input.size();
		const auto b0 = static_cast<unsigned int>(input[i]);
		const auto b1 = has_b1 ? static_cast<unsigned int>(input[i + 1]) : 0U;
		const auto b2 = has_b2 ? static_cast<unsigned int>(input[i + 2]) : 0U;

		output.push_back(kAlphabet[(b0 >> 2) & 0x3fU]);
		output.push_back(kAlphabet[((b0 << 4) | (b1 >> 4)) & 0x3fU]);
		output.push_back(has_b1 ? kAlphabet[((b1 << 2) | (b2 >> 6)) & 0x3fU] : '=');
		output.push_back(has_b2 ? kAlphabet[b2 & 0x3fU] : '=');
	}

	return output;
}

bytes base64_decode(std::string_view input)
{
	reject_invalid_base64(input);

	if (input.empty())
	{
		return {};
	}

	std::size_t padding = 0;
	if (input.ends_with("=="))
	{
		padding = 2;
	}
	else if (input.ends_with("="))
	{
		padding = 1;
	}

	bytes output;
	const std::size_t decoded_groups = input.size() / 4;
	const std::size_t decoded_size = decoded_groups * 3 - padding;
	if (decoded_size > output.max_size())
	{
		throw error(error_code::invalid_input, "base64 input is too large to decode");
	}
	output.reserve(decoded_size);

	for (std::size_t i = 0; i < input.size(); i += 4)
	{
		const int c0 = decode_char(input[i]);
		const int c1 = decode_char(input[i + 1]);
		const int c2 = input[i + 2] == '=' ? 0 : decode_char(input[i + 2]);
		const int c3 = input[i + 3] == '=' ? 0 : decode_char(input[i + 3]);

		output.push_back(static_cast<std::byte>((c0 << 2) | (c1 >> 4)));
		if (input[i + 2] != '=')
		{
			output.push_back(static_cast<std::byte>(((c1 & 0x0f) << 4) | (c2 >> 2)));
		}
		if (input[i + 3] != '=')
		{
			output.push_back(static_cast<std::byte>(((c2 & 0x03) << 6) | c3));
		}
	}

	if (base64_encode(output) != input)
	{
		throw error(error_code::invalid_encoding, "base64 input is not in canonical form");
	}

	return output;
}

} // namespace amv
