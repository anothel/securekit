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

securekit::bytes bytes_from_hex(std::string_view hex)
{
	return securekit::hex_decode(hex);
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

TEST(Hash, HmacSha256Rfc4231TestCase1)
{
	const securekit::bytes key(20, std::byte{0x0b});
	constexpr std::string_view expected = "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";

	EXPECT_EQ(securekit::hmac_sha256(key, bytes_from_ascii("Hi There")), digest_from_hex(expected));
}

TEST(Hash, HmacSha256Rfc4231TestCase2)
{
	const securekit::bytes key = bytes_from_ascii("Jefe");
	constexpr std::string_view expected = "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843";

	EXPECT_EQ(securekit::hmac_sha256(key, bytes_from_ascii("what do ya want for nothing?")), digest_from_hex(expected));
}

TEST(Hash, HkdfSha256Rfc5869TestCase1)
{
	const securekit::bytes key(22, std::byte{0x0b});
	const securekit::bytes salt = bytes_from_hex("000102030405060708090a0b0c");
	const securekit::bytes info = bytes_from_hex("f0f1f2f3f4f5f6f7f8f9");
	constexpr std::string_view expected =
	    "3cb25f25faacd57a90434f64d0362f2a"
	    "2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
	    "34007208d5b887185865";

	EXPECT_EQ(securekit::hkdf_sha256(key, salt, info, 42), bytes_from_hex(expected));
}

TEST(Hash, HkdfSha256Rfc5869TestCase3EmptySaltAndInfo)
{
	const securekit::bytes key(22, std::byte{0x0b});
	const securekit::bytes salt;
	const securekit::bytes info;
	constexpr std::string_view expected =
	    "8da4e775a563c18f715f802a063c5a31"
	    "b8a11f5c5ee1879ec3454e5f3c738d2d"
	    "9d201395faa4b61a96c8";

	EXPECT_EQ(securekit::hkdf_sha256(key, salt, info, 42), bytes_from_hex(expected));
}

TEST(Hash, HkdfSha256ReturnsEmptyOutputWhenRequested)
{
	EXPECT_TRUE(securekit::hkdf_sha256(bytes_from_ascii("key"), {}, {}, 0).empty());
}
