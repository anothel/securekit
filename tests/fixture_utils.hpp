#ifndef SECUREKIT_TEST_FIXTURE_UTILS_HPP_
#define SECUREKIT_TEST_FIXTURE_UTILS_HPP_

#include <cctype>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

#include "securekit/hex.hpp"
#include "securekit/types.hpp"

namespace securekit::test
{

inline bool is_ascii_whitespace(unsigned char ch)
{
	return std::isspace(ch) != 0;
}

inline bool is_lowercase_hex_digit(unsigned char ch)
{
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
}

inline bytes read_hex_fixture(std::string_view name)
{
	const std::filesystem::path path = std::filesystem::path(SECUREKIT_TEST_FIXTURE_DIR) / name;
	std::ifstream in(path);
	if (!in)
	{
		throw std::runtime_error("failed to open fixture: " + path.string());
	}

	const std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	std::string hex;
	hex.reserve(text.size());

	for (std::size_t index = 0; index < text.size(); ++index)
	{
		const auto ch = static_cast<unsigned char>(text[index]);
		if (is_ascii_whitespace(ch))
		{
			continue;
		}
		if (!is_lowercase_hex_digit(ch))
		{
			throw std::runtime_error(
			    "fixture contains non-canonical hex at byte " + std::to_string(index) + ": " + path.string());
		}
		hex.push_back(static_cast<char>(ch));
	}

	if (hex.empty())
	{
		throw std::runtime_error("fixture is empty: " + path.string());
	}
	if ((hex.size() % 2) != 0)
	{
		throw std::runtime_error("fixture has odd hex length: " + path.string());
	}

	try
	{
		return hex_decode(hex);
	}
	catch (const std::exception &e)
	{
		throw std::runtime_error("failed to decode fixture: " + path.string() + ": " + e.what());
	}
}

} // namespace securekit::test

#endif // SECUREKIT_TEST_FIXTURE_UTILS_HPP_
