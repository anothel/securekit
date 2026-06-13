#include "securekit/file.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "securekit/error.hpp"

namespace
{

securekit::key256 key_from_seed(unsigned int seed)
{
	securekit::key256 key{};
	for (std::size_t i = 0; i < key.size(); ++i)
	{
		key[i] = static_cast<std::byte>((seed + i) & 0xffu);
	}
	return key;
}

std::filesystem::path test_path(const std::string &name)
{
	return std::filesystem::temp_directory_path() / ("securekit-" + name);
}

void write_file(const std::filesystem::path &path, const securekit::bytes &bytes)
{
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	ASSERT_TRUE(out);
	out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	ASSERT_TRUE(out);
}

securekit::bytes read_file(const std::filesystem::path &path)
{
	std::ifstream in(path, std::ios::binary);
	EXPECT_TRUE(in);
	const std::string data{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	securekit::bytes out;
	out.reserve(data.size());
	for (const unsigned char ch : data)
	{
		out.push_back(static_cast<std::byte>(ch));
	}
	return out;
}

securekit::bytes bytes_from_text(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (const unsigned char ch : text)
	{
		out.push_back(static_cast<std::byte>(ch));
	}
	return out;
}

template <typename Func>
void expect_error(Func &&func, securekit::error_code expected)
{
	try
	{
		std::forward<Func>(func)();
		FAIL() << "expected securekit::error";
	}
	catch (const securekit::error &ex)
	{
		EXPECT_EQ(ex.code(), expected);
	}
}

void expect_authentication_failed(auto &&func)
{
	expect_error(std::forward<decltype(func)>(func), securekit::error_code::authentication_failed);
}

void expect_invalid_packet(auto &&func)
{
	expect_error(std::forward<decltype(func)>(func), securekit::error_code::invalid_packet);
}

} // namespace

TEST(File, RoundTripsSmallFile)
{
	const auto plain_path = test_path("plain-small.bin");
	const auto sealed_path = test_path("sealed-small.skf");
	const auto opened_path = test_path("opened-small.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::bytes plaintext = bytes_from_text("hello file sealing");
	const securekit::bytes aad = bytes_from_text("file:test");
	const securekit::key256 key = key_from_seed(0x10);
	write_file(plain_path, plaintext);

	securekit::seal_file(plain_path, sealed_path, key, aad);
	const securekit::bytes sealed = read_file(sealed_path);
	ASSERT_GE(sealed.size(), 4u + 1u + 1u + 4u + 32u + 8u + 4u + 4u + 1u + 16u);
	EXPECT_EQ(sealed[0], std::byte{'S'});
	EXPECT_EQ(sealed[1], std::byte{'K'});
	EXPECT_EQ(sealed[2], std::byte{'F'});
	EXPECT_EQ(sealed[3], std::byte{'1'});
	EXPECT_EQ(sealed[4], std::byte{0x01});
	EXPECT_EQ(sealed[5], std::byte{0x01});

	securekit::open_file(sealed_path, opened_path, key, aad);

	EXPECT_EQ(read_file(opened_path), plaintext);
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, RoundTripsEmptyFile)
{
	const auto plain_path = test_path("plain-empty.bin");
	const auto sealed_path = test_path("sealed-empty.skf");
	const auto opened_path = test_path("opened-empty.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::key256 key = key_from_seed(0x21);
	write_file(plain_path, {});
	securekit::seal_file(plain_path, sealed_path, key);
	securekit::open_file(sealed_path, opened_path, key);
	EXPECT_TRUE(read_file(opened_path).empty());

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, RejectsExistingOutput)
{
	const auto plain_path = test_path("plain-output-exists.bin");
	const auto sealed_path = test_path("sealed-output-exists.skf");
	const auto opened_path = test_path("opened-output-exists.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::key256 key = key_from_seed(0x30);
	write_file(plain_path, bytes_from_text("input"));
	write_file(sealed_path, bytes_from_text("existing"));

	expect_error([&] { securekit::seal_file(plain_path, sealed_path, key); }, securekit::error_code::invalid_input);
	EXPECT_EQ(read_file(sealed_path), bytes_from_text("existing"));

	std::filesystem::remove(sealed_path);
	securekit::seal_file(plain_path, sealed_path, key);
	write_file(opened_path, bytes_from_text("existing output"));
	expect_error([&] { securekit::open_file(sealed_path, opened_path, key); }, securekit::error_code::invalid_input);
	EXPECT_EQ(read_file(opened_path), bytes_from_text("existing output"));

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, RejectsMalformedHeader)
{
	const auto plain_path = test_path("plain-malformed-header.bin");
	const auto sealed_path = test_path("sealed-malformed-header.skf");
	const auto opened_path = test_path("opened-malformed-header.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::key256 key = key_from_seed(0x31);
	write_file(plain_path, bytes_from_text("header"));
	securekit::seal_file(plain_path, sealed_path, key);

	for (const std::size_t offset : {0u, 4u, 5u, 9u})
	{
		securekit::bytes sealed = read_file(sealed_path);
		sealed[offset] ^= std::byte{0x01};
		write_file(sealed_path, sealed);
		expect_error([&] { securekit::open_file(sealed_path, opened_path, key); }, securekit::error_code::invalid_packet);
		std::filesystem::remove(opened_path);
	}

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, RejectsWrongKeyAndAad)
{
	const auto plain_path = test_path("plain-wrong-key.bin");
	const auto sealed_path = test_path("sealed-wrong-key.skf");
	const auto opened_path = test_path("opened-wrong-key.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::key256 key = key_from_seed(0x40);
	const securekit::key256 wrong_key = key_from_seed(0x41);
	const securekit::bytes aad = bytes_from_text("aad");
	write_file(plain_path, bytes_from_text("secret"));
	securekit::seal_file(plain_path, sealed_path, key, aad);

	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, wrong_key, aad); });
	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, key, bytes_from_text("wrong aad")); });

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, DetectsMutation)
{
	const auto plain_path = test_path("plain-mutation.bin");
	const auto sealed_path = test_path("sealed-mutation.skf");
	const auto opened_path = test_path("opened-mutation.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	const securekit::key256 key = key_from_seed(0x50);
	write_file(plain_path, bytes_from_text("mutable"));
	securekit::seal_file(plain_path, sealed_path, key);
	const securekit::bytes original = read_file(sealed_path);

	const std::size_t header_size = 50;
	const std::size_t record_size_offset = header_size + 4;
	const std::size_t record_flag_offset = header_size + 8;
	const std::size_t ciphertext_offset = header_size + 9;
	const std::size_t tag_offset = original.size() - 16;

	securekit::bytes mutated_header = original;
	mutated_header[10] ^= std::byte{0x01};
	write_file(sealed_path, mutated_header);
	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, key); });

	securekit::bytes mutated_size = original;
	mutated_size[record_size_offset + 3] ^= std::byte{0x01};
	write_file(sealed_path, mutated_size);
	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, key); });

	securekit::bytes mutated_flag = original;
	mutated_flag[record_flag_offset] = std::byte{0x00};
	write_file(sealed_path, mutated_flag);
	expect_invalid_packet([&] { securekit::open_file(sealed_path, opened_path, key); });

	securekit::bytes mutated_ciphertext = original;
	mutated_ciphertext[ciphertext_offset] ^= std::byte{0x01};
	write_file(sealed_path, mutated_ciphertext);
	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, key); });

	securekit::bytes mutated_tag = original;
	mutated_tag[tag_offset] ^= std::byte{0x01};
	write_file(sealed_path, mutated_tag);
	expect_authentication_failed([&] { securekit::open_file(sealed_path, opened_path, key); });

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, DetectsTruncationAppendAndReorder)
{
	const auto plain_path = test_path("plain-shape.bin");
	const auto sealed_path = test_path("sealed-shape.skf");
	const auto opened_path = test_path("opened-shape.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	securekit::bytes plaintext((1024u * 1024u * 2u) + 17u);
	for (std::size_t i = 0; i < plaintext.size(); ++i)
	{
		plaintext[i] = static_cast<std::byte>((i * 13u) & 0xffu);
	}

	const securekit::key256 key = key_from_seed(0x60);
	write_file(plain_path, plaintext);
	securekit::seal_file(plain_path, sealed_path, key);
	const securekit::bytes original = read_file(sealed_path);

	securekit::bytes truncated = original;
	truncated.pop_back();
	write_file(sealed_path, truncated);
	expect_invalid_packet([&] { securekit::open_file(sealed_path, opened_path, key); });

	securekit::bytes appended = original;
	appended.push_back(std::byte{0x00});
	write_file(sealed_path, appended);
	expect_invalid_packet([&] { securekit::open_file(sealed_path, opened_path, key); });

	const std::size_t header_size = 50;
	const std::size_t full_record_size = 9 + (1024u * 1024u) + 16;
	securekit::bytes reordered = original;
	std::swap_ranges(
	    reordered.begin() + static_cast<std::ptrdiff_t>(header_size),
	    reordered.begin() + static_cast<std::ptrdiff_t>(header_size + full_record_size),
	    reordered.begin() + static_cast<std::ptrdiff_t>(header_size + full_record_size));
	write_file(sealed_path, reordered);
	expect_invalid_packet([&] { securekit::open_file(sealed_path, opened_path, key); });

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}

TEST(File, RoundTripsMultiChunkFile)
{
	const auto plain_path = test_path("plain-large.bin");
	const auto sealed_path = test_path("sealed-large.skf");
	const auto opened_path = test_path("opened-large.bin");
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	securekit::bytes plaintext((1024u * 1024u * 2u) + 17u);
	for (std::size_t i = 0; i < plaintext.size(); ++i)
	{
		plaintext[i] = static_cast<std::byte>((i * 31u) & 0xffu);
	}

	const securekit::key256 key = key_from_seed(0x20);
	write_file(plain_path, plaintext);
	securekit::seal_file(plain_path, sealed_path, key);
	securekit::open_file(sealed_path, opened_path, key);
	EXPECT_EQ(read_file(opened_path), plaintext);

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);
}
