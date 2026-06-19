#include "securekit/aead.hpp"
#include "securekit/error.hpp"
#include "securekit/packet_stream.hpp"

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

namespace
{

constexpr std::size_t kHeaderSize = 5;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kPrefixSize = kHeaderSize + kNonceSize;
constexpr std::size_t kOverhead = kPrefixSize + kTagSize;

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
void expect_invalid_input(Func &&func)
{
	expect_error(std::forward<Func>(func), securekit::error_code::invalid_input);
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

securekit::bytes join(std::initializer_list<securekit::bytes> parts)
{
	securekit::bytes out;
	std::size_t total_size = 0;
	for (const securekit::bytes &part : parts)
	{
		total_size += part.size();
	}

	out.reserve(total_size);
	for (const securekit::bytes &part : parts)
	{
		out.insert(out.end(), part.begin(), part.end());
	}
	return out;
}

} // namespace

TEST(PacketStream, EncryptsWithChunkedUpdatesAndRoundTrips)
{
	const securekit::key256 key = key_from_seed(0x10);
	const securekit::bytes aad = bytes_from_ascii("record:v1");
	const securekit::bytes plaintext = bytes_from_ascii("streaming packet plaintext");

	securekit::packet_encryptor encryptor(key, aad);
	const securekit::bytes prefix = encryptor.begin();
	const securekit::bytes part1 = encryptor.update(std::span<const std::byte>(plaintext).first(7));
	const securekit::bytes part2 = encryptor.update(std::span<const std::byte>(plaintext).subspan(7, 8));
	const securekit::bytes part3 = encryptor.update(std::span<const std::byte>(plaintext).subspan(15));
	const securekit::bytes tag = encryptor.finalize();
	const securekit::bytes packet = join({prefix, part1, part2, part3, tag});

	ASSERT_EQ(prefix.size(), kPrefixSize);
	ASSERT_EQ(tag.size(), kTagSize);
	ASSERT_EQ(packet.size(), plaintext.size() + kOverhead);
	EXPECT_EQ(packet[0], std::byte{'S'});
	EXPECT_EQ(packet[1], std::byte{'K'});
	EXPECT_EQ(packet[2], std::byte{'T'});
	EXPECT_EQ(packet[3], std::byte{'1'});
	EXPECT_EQ(packet[4], std::byte{0x01});
	EXPECT_EQ(securekit::decrypt(packet, key, aad), plaintext);
}

TEST(PacketStream, DecryptsOneShotPacketWithChunkedUpdates)
{
	const securekit::key256 key = key_from_seed(0x20);
	const securekit::bytes aad = bytes_from_ascii("aad");
	const securekit::bytes plaintext = bytes_from_values({
	    0x00,
	    0x01,
	    0x02,
	    0x7f,
	    0x80,
	    0xfe,
	    0xff,
	    0x42,
	    0x43,
	});
	const securekit::bytes packet = securekit::encrypt(plaintext, key, aad);

	const std::span<const std::byte> packet_span(packet);
	const std::span<const std::byte> prefix = packet_span.first(kPrefixSize);
	const std::span<const std::byte> ciphertext = packet_span.subspan(kPrefixSize, plaintext.size());
	const std::span<const std::byte> tag = packet_span.last(kTagSize);

	securekit::packet_decryptor decryptor(key, aad);
	decryptor.begin(prefix);
	const securekit::bytes part1 = decryptor.update(ciphertext.first(2));
	const securekit::bytes part2 = decryptor.update(ciphertext.subspan(2, 3));
	const securekit::bytes part3 = decryptor.update(ciphertext.subspan(5));
	const securekit::bytes tail = decryptor.finalize(tag);

	EXPECT_EQ(join({part1, part2, part3, tail}), plaintext);
	EXPECT_TRUE(tail.empty());
}

TEST(PacketStream, DecryptsKnownPacketVectorWithChunkedUpdates)
{
	const securekit::key256 key{};
	const securekit::bytes aad = bytes_from_ascii("record:v1");
	const securekit::bytes expected_plaintext = bytes_from_ascii("hello securekit");
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

	const std::span<const std::byte> packet_span(packet);
	const std::span<const std::byte> ciphertext = packet_span.subspan(kPrefixSize, expected_plaintext.size());

	securekit::packet_decryptor decryptor(key, aad);
	decryptor.begin(packet_span.first(kPrefixSize));
	const securekit::bytes part1 = decryptor.update(ciphertext.first(5));
	const securekit::bytes part2 = decryptor.update(ciphertext.subspan(5));
	const securekit::bytes tail = decryptor.finalize(packet_span.last(kTagSize));

	EXPECT_EQ(join({part1, part2, tail}), expected_plaintext);
	EXPECT_TRUE(tail.empty());
}

TEST(PacketStream, RoundTripsEmptyPlaintext)
{
	const securekit::key256 key = key_from_seed(0x30);

	securekit::packet_encryptor encryptor(key);
	const securekit::bytes packet = join({encryptor.begin(), encryptor.finalize()});

	ASSERT_EQ(packet.size(), kOverhead);
	EXPECT_TRUE(securekit::decrypt(packet, key).empty());

	securekit::packet_decryptor decryptor(key);
	decryptor.begin(std::span<const std::byte>(packet).first(kPrefixSize));
	EXPECT_TRUE(decryptor.update(std::span<const std::byte>{}).empty());
	EXPECT_TRUE(decryptor.finalize(std::span<const std::byte>(packet).last(kTagSize)).empty());
}

TEST(PacketStream, RejectsEncryptInvalidCallOrder)
{
	const securekit::key256 key = key_from_seed(0x40);
	securekit::packet_encryptor encryptor(key);

	expect_invalid_input([&] { (void)encryptor.update(bytes_from_ascii("abc")); });
	expect_invalid_input([&] { (void)encryptor.finalize(); });

	(void)encryptor.begin();
	expect_invalid_input([&] { (void)encryptor.begin(); });

	(void)encryptor.finalize();
	expect_invalid_input([&] { (void)encryptor.update(bytes_from_ascii("abc")); });
	expect_invalid_input([&] { (void)encryptor.finalize(); });
}

TEST(PacketStream, RejectsDecryptInvalidCallOrder)
{
	const securekit::key256 key = key_from_seed(0x50);
	securekit::packet_decryptor decryptor(key);

	expect_invalid_input([&] { (void)decryptor.update(bytes_from_ascii("abc")); });
	expect_invalid_input([&] { (void)decryptor.finalize(bytes_from_values({0x00})); });

	const securekit::bytes packet = securekit::encrypt(bytes_from_ascii("message"), key);
	decryptor.begin(std::span<const std::byte>(packet).first(kPrefixSize));
	expect_invalid_input([&] { decryptor.begin(std::span<const std::byte>(packet).first(kPrefixSize)); });

	(void)decryptor.update(std::span<const std::byte>(packet).subspan(kPrefixSize, packet.size() - kOverhead));
	(void)decryptor.finalize(std::span<const std::byte>(packet).last(kTagSize));

	expect_invalid_input([&] { (void)decryptor.update(bytes_from_ascii("abc")); });
	expect_invalid_input([&] { (void)decryptor.finalize(std::span<const std::byte>(packet).last(kTagSize)); });
}

TEST(PacketStream, RejectsMalformedPrefixAndTag)
{
	const securekit::key256 key = key_from_seed(0x60);
	const securekit::bytes packet = securekit::encrypt(bytes_from_ascii("message"), key);
	const std::span<const std::byte> packet_span(packet);

	securekit::bytes bad_magic(packet_span.first(kPrefixSize).begin(), packet_span.first(kPrefixSize).end());
	bad_magic[0] = std::byte{'X'};

	securekit::bytes bad_version(packet_span.first(kPrefixSize).begin(), packet_span.first(kPrefixSize).end());
	bad_version[kHeaderSize - 1] = std::byte{0x02};

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(packet_span.first(kPrefixSize - 1));
	});

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(packet_span.first(kPrefixSize + 1));
	});

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(bad_magic);
	});

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(bad_version);
	});

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(packet_span.first(kPrefixSize));
		(void)decryptor.finalize(packet_span.last(kTagSize - 1));
	});

	expect_invalid_packet([&] {
		securekit::packet_decryptor decryptor(key);
		decryptor.begin(packet_span.first(kPrefixSize));
		(void)decryptor.finalize(bytes_from_values({
		    0x00,
		    0x01,
		    0x02,
		    0x03,
		    0x04,
		    0x05,
		    0x06,
		    0x07,
		    0x08,
		    0x09,
		    0x0a,
		    0x0b,
		    0x0c,
		    0x0d,
		    0x0e,
		    0x0f,
		    0x10,
		}));
	});
}

TEST(PacketStream, RejectsWrongKeyAndAadAtFinalize)
{
	const securekit::key256 key = key_from_seed(0x70);
	const securekit::key256 wrong_key = key_from_seed(0x71);
	const securekit::bytes aad = bytes_from_ascii("aad");
	const securekit::bytes wrong_aad = bytes_from_ascii("other aad");
	const securekit::bytes plaintext = bytes_from_ascii("secret");
	const securekit::bytes packet = securekit::encrypt(plaintext, key, aad);
	const std::span<const std::byte> packet_span(packet);
	const std::span<const std::byte> prefix = packet_span.first(kPrefixSize);
	const std::span<const std::byte> ciphertext = packet_span.subspan(kPrefixSize, plaintext.size());
	const std::span<const std::byte> tag = packet_span.last(kTagSize);

	expect_authentication_failed([&] {
		securekit::packet_decryptor decryptor(wrong_key, aad);
		decryptor.begin(prefix);
		(void)decryptor.update(ciphertext);
		(void)decryptor.finalize(tag);
	});

	expect_authentication_failed([&] {
		securekit::packet_decryptor decryptor(key, wrong_aad);
		decryptor.begin(prefix);
		(void)decryptor.update(ciphertext);
		(void)decryptor.finalize(tag);
	});
}
