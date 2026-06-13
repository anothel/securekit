#include "securekit/aead.hpp"

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "securekit/error.hpp"

namespace
{

constexpr std::size_t kHeaderSize = 5;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kOverhead = kHeaderSize + kNonceSize + kTagSize;

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

securekit::bytes bytes_from_values(std::initializer_list<unsigned int> values)
{
	securekit::bytes out;
	out.reserve(values.size());
	for (unsigned int value : values)
	{
		out.push_back(static_cast<std::byte>(value));
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

template <typename Func>
void expect_invalid_packet(Func &&func)
{
	expect_error(std::forward<Func>(func), securekit::error_code::invalid_packet);
}

template <typename Func>
void expect_authentication_failed(Func &&func)
{
	expect_error(std::forward<Func>(func), securekit::error_code::authentication_failed);
}

template <typename Func>
void expect_generic_authentication_failure(Func &&func)
{
	try
	{
		std::forward<Func>(func)();
		FAIL() << "expected securekit::error";
	}
	catch (const securekit::error &e)
	{
		EXPECT_EQ(e.code(), securekit::error_code::authentication_failed);
		EXPECT_STREQ(e.what(), "AEAD authentication failed");
	}
}

} // namespace

TEST(Aead, RoundTripsBinaryPlaintext)
{
	const securekit::key256 key = key_from_seed(0x10);
	const securekit::bytes plaintext = bytes_from_values({
	    0x00,
	    0x01,
	    0x02,
	    0x7f,
	    0x80,
	    0xff,
	    0x00,
	    0x42,
	});

	const securekit::bytes packet = securekit::encrypt(plaintext, key);

	ASSERT_EQ(packet.size(), plaintext.size() + kOverhead);
	EXPECT_EQ(packet[0], std::byte{'S'});
	EXPECT_EQ(packet[1], std::byte{'K'});
	EXPECT_EQ(packet[2], std::byte{'T'});
	EXPECT_EQ(packet[3], std::byte{'1'});
	EXPECT_EQ(packet[4], std::byte{0x01});
	EXPECT_EQ(securekit::decrypt(packet, key), plaintext);
}

TEST(Aead, RoundTripsEmptyPlaintext)
{
	const securekit::key256 key = key_from_seed(0x20);
	const securekit::bytes plaintext;

	const securekit::bytes packet = securekit::encrypt(plaintext, key);

	EXPECT_EQ(packet.size(), kOverhead);
	EXPECT_TRUE(securekit::decrypt(packet, key).empty());
}

TEST(Aead, AuthenticatesAdditionalData)
{
	const securekit::key256 key = key_from_seed(0x30);
	const securekit::bytes plaintext = bytes_from_ascii("authenticated message");
	const securekit::bytes aad = bytes_from_ascii("context");
	const securekit::bytes wrong_aad = bytes_from_ascii("other context");

	const securekit::bytes packet = securekit::encrypt(plaintext, key, aad);

	EXPECT_EQ(securekit::decrypt(packet, key, aad), plaintext);
	expect_authentication_failed([&] { (void)securekit::decrypt(packet, key, wrong_aad); });
}

TEST(Aead, AuthenticationFailuresUseGenericMessage)
{
	const securekit::key256 key = key_from_seed(0x70);
	const securekit::key256 wrong_key = key_from_seed(0x71);
	const securekit::bytes plaintext = bytes_from_ascii("secret");
	const securekit::bytes aad = bytes_from_ascii("aad");
	const securekit::bytes wrong_aad = bytes_from_ascii("other aad");

	const securekit::bytes packet = securekit::encrypt(plaintext, key, aad);

	securekit::bytes bad_nonce = packet;
	bad_nonce[kHeaderSize] ^= std::byte{0x01};

	securekit::bytes bad_ciphertext = packet;
	bad_ciphertext[kHeaderSize + kNonceSize] ^= std::byte{0x01};

	securekit::bytes bad_tag = packet;
	bad_tag[bad_tag.size() - 1] ^= std::byte{0x01};

	expect_generic_authentication_failure([&] { (void)securekit::decrypt(packet, key, wrong_aad); });
	expect_generic_authentication_failure([&] { (void)securekit::decrypt(packet, wrong_key, aad); });
	expect_generic_authentication_failure([&] { (void)securekit::decrypt(bad_nonce, key, aad); });
	expect_generic_authentication_failure([&] { (void)securekit::decrypt(bad_ciphertext, key, aad); });
	expect_generic_authentication_failure([&] { (void)securekit::decrypt(bad_tag, key, aad); });
}

TEST(Aead, DecryptsKnownAes256GcmPacketVector)
{
	const securekit::key256 key{};
	const securekit::bytes aad = bytes_from_ascii("record:v1");
	const securekit::bytes expected_plaintext = bytes_from_ascii("hello securekit");

	// AES-256-GCM vector with all-zero key/nonce and AAD = "SKT1\x01record:v1".
	const securekit::bytes packet = bytes_from_values({
	    0x53,
	    0x4b,
	    0x54,
	    0x31,
	    0x01,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0xa6,
	    0xc2,
	    0x2c,
	    0x51,
	    0x22,
	    0x40,
	    0x18,
	    0x0b,
	    0x64,
	    0x3b,
	    0xb7,
	    0xb6,
	    0xd1,
	    0x9a,
	    0xe9,
	    0x1d,
	    0x51,
	    0xdb,
	    0x38,
	    0x76,
	    0x93,
	    0xb2,
	    0xf1,
	    0x65,
	    0x22,
	    0x06,
	    0x13,
	    0xf9,
	    0x87,
	    0x28,
	    0xde,
	});

	EXPECT_EQ(securekit::decrypt(packet, key, aad), expected_plaintext);
}

TEST(Aead, RejectsInvalidPacketShape)
{
	const securekit::key256 key = key_from_seed(0x40);
	const securekit::bytes plaintext = bytes_from_ascii("message");
	const securekit::bytes packet = securekit::encrypt(plaintext, key);

	expect_invalid_packet([&] { (void)securekit::decrypt(securekit::bytes(kOverhead - 1), key); });

	securekit::bytes bad_magic = packet;
	bad_magic[0] = std::byte{'X'};
	expect_invalid_packet([&] { (void)securekit::decrypt(bad_magic, key); });

	securekit::bytes bad_version = packet;
	bad_version[4] = std::byte{0x02};
	expect_invalid_packet([&] { (void)securekit::decrypt(bad_version, key); });
}

TEST(Aead, DetectsPacketMutation)
{
	const securekit::key256 key = key_from_seed(0x50);
	const securekit::bytes plaintext = bytes_from_ascii("mutable ciphertext");
	const securekit::bytes aad = bytes_from_ascii("aad");
	const securekit::bytes packet = securekit::encrypt(plaintext, key, aad);

	securekit::bytes bad_nonce = packet;
	bad_nonce[kHeaderSize] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)securekit::decrypt(bad_nonce, key, aad); });

	securekit::bytes bad_ciphertext = packet;
	bad_ciphertext[kHeaderSize + kNonceSize] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)securekit::decrypt(bad_ciphertext, key, aad); });

	securekit::bytes bad_tag = packet;
	bad_tag[bad_tag.size() - 1] ^= std::byte{0x01};
	expect_authentication_failed([&] { (void)securekit::decrypt(bad_tag, key, aad); });
}

TEST(Aead, RejectsWrongKey)
{
	const securekit::key256 key = key_from_seed(0x60);
	const securekit::key256 wrong_key = key_from_seed(0x61);
	const securekit::bytes plaintext = bytes_from_ascii("secret");

	const securekit::bytes packet = securekit::encrypt(plaintext, key);

	expect_authentication_failed([&] { (void)securekit::decrypt(packet, wrong_key); });
}
