#include "securekit/key_wrap.hpp"

#include <cstddef>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "securekit/aead.hpp"
#include "securekit/error.hpp"
#include "fixture_utils.hpp"

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
	const securekit::bytes packet = securekit::test::read_hex_fixture("skt1-key-wrap.hex");

	EXPECT_EQ(securekit::unwrap_key(packet, wrapping_key, aad), key_to_wrap);
}

TEST(KeyWrap, UnwrapsKnownZeroKeyWrapFixture)
{
	const securekit::key256 key_to_wrap{};
	const securekit::key256 wrapping_key = key_from_seed(0x80);
	const securekit::bytes packet = securekit::test::read_hex_fixture("skt1-key-wrap-zero.hex");

	EXPECT_EQ(securekit::unwrap_key(packet, wrapping_key), key_to_wrap);
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
