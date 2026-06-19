#include "securekit/key_wrap.hpp"

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "securekit/aead.hpp"
#include "securekit/error.hpp"

namespace
{

securekit::bytes bytes_from_ascii(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (const char ch : text)
	{
		out.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
	}
	return out;
}

securekit::bytes bytes_from_values(std::initializer_list<unsigned int> values)
{
	securekit::bytes out;
	out.reserve(values.size());
	for (const unsigned int value : values)
	{
		out.push_back(static_cast<std::byte>(value & 0xffu));
	}
	return out;
}

securekit::key256 key_from_seed(unsigned int seed)
{
	securekit::key256 key{};
	for (std::size_t i = 0; i < key.size(); ++i)
	{
		key[i] = static_cast<std::byte>((seed + i) & 0xffu);
	}
	return key;
}

template <typename Func>
void expect_error(Func &&func, securekit::error_code expected)
{
	try
	{
		std::forward<Func>(func)();
		FAIL() << "expected securekit::error";
	}
	catch (const securekit::error &e)
	{
		EXPECT_EQ(e.code(), expected);
	}
}

} // namespace

TEST(KeyWrap, WrapsAndUnwrapsKeyWithSkt1Packet)
{
	const securekit::key256 key_to_wrap = key_from_seed(0x10);
	const securekit::key256 wrapping_key = key_from_seed(0x40);
	const securekit::bytes aad = bytes_from_ascii("key-id:primary");

	const securekit::bytes packet = securekit::wrap_key(key_to_wrap, wrapping_key, aad);

	ASSERT_GE(packet.size(), 5u);
	EXPECT_EQ(packet[0], std::byte{'S'});
	EXPECT_EQ(packet[1], std::byte{'K'});
	EXPECT_EQ(packet[2], std::byte{'T'});
	EXPECT_EQ(packet[3], std::byte{'1'});
	EXPECT_EQ(packet[4], std::byte{0x01});
	EXPECT_EQ(securekit::unwrap_key(packet, wrapping_key, aad), key_to_wrap);
}

TEST(KeyWrap, UnwrapsKnownSkt1KeyWrapFixture)
{
	const securekit::key256 key_to_wrap = key_from_seed(0x10);
	const securekit::key256 wrapping_key = key_from_seed(0x40);
	const securekit::bytes aad = bytes_from_ascii("key-id:primary");
	const securekit::bytes packet = bytes_from_values({
	    0x53,
	    0x4b,
	    0x54,
	    0x31,
	    0x01,
	    0xf0,
	    0xf1,
	    0xf2,
	    0xf3,
	    0xf4,
	    0xf5,
	    0xf6,
	    0xf7,
	    0xf8,
	    0xf9,
	    0xfa,
	    0xfb,
	    0xc3,
	    0x2c,
	    0x64,
	    0x11,
	    0xeb,
	    0x47,
	    0xc7,
	    0xef,
	    0x68,
	    0xa4,
	    0x82,
	    0xe2,
	    0xb1,
	    0xa5,
	    0xb0,
	    0x45,
	    0xcf,
	    0x56,
	    0x90,
	    0x78,
	    0x5e,
	    0xd1,
	    0x77,
	    0x24,
	    0x7c,
	    0x1e,
	    0xef,
	    0xb0,
	    0xa0,
	    0xa4,
	    0x6a,
	    0x41,
	    0x99,
	    0xa8,
	    0xc2,
	    0xeb,
	    0x3e,
	    0xd2,
	    0xb9,
	    0x65,
	    0x6a,
	    0x83,
	    0x2a,
	    0x88,
	    0x4c,
	    0x51,
	    0x62,
	    0x09,
	});

	EXPECT_EQ(securekit::unwrap_key(packet, wrapping_key, aad), key_to_wrap);
}

TEST(KeyWrap, RejectsWrongWrappingKey)
{
	const securekit::key256 key_to_wrap = key_from_seed(0x20);
	const securekit::key256 wrapping_key = key_from_seed(0x50);
	const securekit::key256 wrong_wrapping_key = key_from_seed(0x51);

	const securekit::bytes packet = securekit::wrap_key(key_to_wrap, wrapping_key);

	expect_error([&] { (void)securekit::unwrap_key(packet, wrong_wrapping_key); }, securekit::error_code::authentication_failed);
}

TEST(KeyWrap, RejectsWrongAad)
{
	const securekit::key256 key_to_wrap = key_from_seed(0x30);
	const securekit::key256 wrapping_key = key_from_seed(0x60);
	const securekit::bytes aad = bytes_from_ascii("key-id:primary");
	const securekit::bytes wrong_aad = bytes_from_ascii("key-id:secondary");

	const securekit::bytes packet = securekit::wrap_key(key_to_wrap, wrapping_key, aad);

	expect_error([&] { (void)securekit::unwrap_key(packet, wrapping_key, wrong_aad); }, securekit::error_code::authentication_failed);
}

TEST(KeyWrap, RejectsPacketsThatDoNotDecryptToOneKey)
{
	const securekit::key256 wrapping_key = key_from_seed(0x70);
	const securekit::bytes packet = securekit::encrypt(bytes_from_ascii("short"), wrapping_key);

	expect_error([&] { (void)securekit::unwrap_key(packet, wrapping_key); }, securekit::error_code::invalid_packet);
}
