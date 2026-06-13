#include "amv/aead.hpp"

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "amv/error.hpp"

namespace
{

constexpr std::size_t kHeaderSize = 5;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kOverhead = kHeaderSize + kNonceSize + kTagSize;

amv::bytes bytes_from_ascii(std::string_view text)
{
	amv::bytes out;
	out.reserve(text.size());
	for (char ch : text)
	{
		out.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
	}
	return out;
}

amv::bytes bytes_from_values(std::initializer_list<unsigned int> values)
{
	amv::bytes out;
	out.reserve(values.size());
	for (unsigned int value : values)
	{
		out.push_back(static_cast<std::byte>(value));
	}
	return out;
}

amv::key256 key_from_seed(unsigned int seed)
{
	amv::key256 key{};
	for (std::size_t i = 0; i < key.size(); ++i)
	{
		key[i] = static_cast<std::byte>((seed + i) & 0xffu);
	}
	return key;
}

template <typename Func>
void expect_error(Func &&func, amv::error_code expected)
{
	try
	{
		std::forward<Func>(func)();
		FAIL() << "expected amv::error";
	}
	catch (const amv::error &e)
	{
		EXPECT_EQ(e.code(), expected);
	}
}

template <typename Func>
void expect_invalid_packet(Func &&func)
{
	expect_error(std::forward<Func>(func), amv::error_code::invalid_packet);
}

template <typename Func>
void expect_authentication_failed(Func &&func)
{
	expect_error(std::forward<Func>(func), amv::error_code::authentication_failed);
}

} // namespace

TEST(Aead, RoundTripsBinaryPlaintext)
{
	const amv::key256 key = key_from_seed(0x10);
	const amv::bytes plaintext = bytes_from_values({
	    0x00,
	    0x01,
	    0x02,
	    0x7f,
	    0x80,
	    0xff,
	    0x00,
	    0x42,
	});

	const amv::bytes packet = amv::encrypt(plaintext, key);

	ASSERT_EQ(packet.size(), plaintext.size() + kOverhead);
	EXPECT_EQ(packet[0], std::byte{'A'});
	EXPECT_EQ(packet[1], std::byte{'M'});
	EXPECT_EQ(packet[2], std::byte{'V'});
	EXPECT_EQ(packet[3], std::byte{'1'});
	EXPECT_EQ(packet[4], std::byte{0x01});
	EXPECT_EQ(amv::decrypt(packet, key), plaintext);
}

TEST(Aead, RoundTripsEmptyPlaintext)
{
	const amv::key256 key = key_from_seed(0x20);
	const amv::bytes plaintext;

	const amv::bytes packet = amv::encrypt(plaintext, key);

	EXPECT_EQ(packet.size(), kOverhead);
	EXPECT_TRUE(amv::decrypt(packet, key).empty());
}

TEST(Aead, AuthenticatesAdditionalData)
{
	const amv::key256 key = key_from_seed(0x30);
	const amv::bytes plaintext = bytes_from_ascii("authenticated message");
	const amv::bytes aad = bytes_from_ascii("context");
	const amv::bytes wrong_aad = bytes_from_ascii("other context");

	const amv::bytes packet = amv::encrypt(plaintext, key, aad);

	EXPECT_EQ(amv::decrypt(packet, key, aad), plaintext);
	expect_authentication_failed([&] { (void)amv::decrypt(packet, key, wrong_aad); });
}

TEST(Aead, RejectsInvalidPacketShape)
{
	const amv::key256 key = key_from_seed(0x40);
	const amv::bytes plaintext = bytes_from_ascii("message");
	const amv::bytes packet = amv::encrypt(plaintext, key);

	expect_invalid_packet([&] { (void)amv::decrypt(amv::bytes(kOverhead - 1), key); });

	amv::bytes bad_magic = packet;
	bad_magic[0] = std::byte{'X'};
	expect_invalid_packet([&] { (void)amv::decrypt(bad_magic, key); });

	amv::bytes bad_version = packet;
	bad_version[4] = std::byte{0x02};
	expect_invalid_packet([&] { (void)amv::decrypt(bad_version, key); });
}

TEST(Aead, DetectsPacketMutation)
{
	const amv::key256 key = key_from_seed(0x50);
	const amv::bytes plaintext = bytes_from_ascii("mutable ciphertext");
	const amv::bytes aad = bytes_from_ascii("aad");
	const amv::bytes packet = amv::encrypt(plaintext, key, aad);

	amv::bytes bad_nonce = packet;
	bad_nonce[kHeaderSize] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)amv::decrypt(bad_nonce, key, aad); });

	amv::bytes bad_ciphertext = packet;
	bad_ciphertext[kHeaderSize + kNonceSize] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)amv::decrypt(bad_ciphertext, key, aad); });

	amv::bytes bad_tag = packet;
	bad_tag[bad_tag.size() - 1] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)amv::decrypt(bad_tag, key, aad); });
}

TEST(Aead, RejectsWrongKey)
{
	const amv::key256 key = key_from_seed(0x60);
	const amv::key256 wrong_key = key_from_seed(0x61);
	const amv::bytes plaintext = bytes_from_ascii("secret");

	const amv::bytes packet = amv::encrypt(plaintext, key);

	expect_authentication_failed([&] { (void)amv::decrypt(packet, wrong_key); });
}
