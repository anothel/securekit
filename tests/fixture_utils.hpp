#ifndef SECUREKIT_TEST_FIXTURE_UTILS_HPP_
#define SECUREKIT_TEST_FIXTURE_UTILS_HPP_

#include <algorithm>
#include <cctype>
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

inline bytes read_hex_fixture(std::string_view name)
{
	const std::filesystem::path path = std::filesystem::path(SECUREKIT_TEST_FIXTURE_DIR) / name;
	std::ifstream in(path);
	if (!in)
	{
		throw std::runtime_error("failed to open fixture: " + path.string());
	}

	std::string hex{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	hex.erase(std::remove_if(hex.begin(), hex.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), hex.end());
	return hex_decode(hex);
}

} // namespace securekit::test

#endif // SECUREKIT_TEST_FIXTURE_UTILS_HPP_
