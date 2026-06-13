#include "securekit/hash.hpp"

#include <algorithm>
#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "securekit/hex.hpp"

namespace
{

securekit::bytes bytes_from_ascii(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (char ch : text)
	{
		out.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
	}
	return out;
}

securekit::digest256 digest_from_hex(std::string_view hex)
{
	const securekit::bytes decoded = securekit::hex_decode(hex);
	securekit::digest256 digest{};
	EXPECT_EQ(decoded.size(), digest.size());
	std::copy(decoded.begin(), decoded.end(), digest.begin());
	return digest;
}

} // namespace

TEST(Hash, Sha256EmptyStringVector)
{
	constexpr std::string_view expected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

	EXPECT_EQ(securekit::sha256(bytes_from_ascii("")), digest_from_hex(expected));
}

TEST(Hash, Sha256AbcVector)
{
	constexpr std::string_view expected = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

	EXPECT_EQ(securekit::sha256(bytes_from_ascii("abc")), digest_from_hex(expected));
}
