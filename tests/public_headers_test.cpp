#include "securekit/aead.hpp"
#include "securekit/base64.hpp"
#include "securekit/compare.hpp"
#include "securekit/error.hpp"
#include "securekit/file.hpp"
#include "securekit/hash.hpp"
#include "securekit/hex.hpp"
#include "securekit/key_wrap.hpp"
#include "securekit/packet_stream.hpp"
#include "securekit/random.hpp"
#include "securekit/securekit.hpp"
#include "securekit/types.hpp"

#include <filesystem>
#include <span>
#include <type_traits>

#include <gtest/gtest.h>

TEST(PublicHeaders, TypeAliasesAreAvailable)
{
	securekit::bytes data;
	securekit::key256 key{};
	securekit::digest256 digest{};

	EXPECT_TRUE(data.empty());
	EXPECT_EQ(key.size(), 32u);
	EXPECT_EQ(digest.size(), 32u);
}

TEST(PublicHeaders, FileApiIsAvailable)
{
	static_assert(std::is_same_v<decltype(&securekit::seal_file), void (*)(const std::filesystem::path &, const std::filesystem::path &, const securekit::key256 &, std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::open_file), void (*)(const std::filesystem::path &, const std::filesystem::path &, const securekit::key256 &, std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::seal_file_with_password), void (*)(const std::filesystem::path &, const std::filesystem::path &, std::span<const std::byte>, std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::open_file_with_password), void (*)(const std::filesystem::path &, const std::filesystem::path &, std::span<const std::byte>, std::span<const std::byte>)>);
}

TEST(PublicHeaders, KeyWrapApiIsAvailable)
{
	static_assert(std::is_same_v<decltype(&securekit::wrap_key), securekit::bytes (*)(const securekit::key256 &, const securekit::key256 &, std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::unwrap_key), securekit::key256 (*)(std::span<const std::byte>, const securekit::key256 &, std::span<const std::byte>)>);
}

TEST(PublicHeaders, PacketStreamApiIsAvailable)
{
	static_assert(!std::is_copy_constructible_v<securekit::packet_encryptor>);
	static_assert(!std::is_copy_assignable_v<securekit::packet_encryptor>);
	static_assert(std::is_move_constructible_v<securekit::packet_encryptor>);
	static_assert(std::is_move_assignable_v<securekit::packet_encryptor>);

	static_assert(!std::is_copy_constructible_v<securekit::packet_decryptor>);
	static_assert(!std::is_copy_assignable_v<securekit::packet_decryptor>);
	static_assert(std::is_move_constructible_v<securekit::packet_decryptor>);
	static_assert(std::is_move_assignable_v<securekit::packet_decryptor>);

	static_assert(std::is_same_v<decltype(&securekit::packet_encryptor::begin), securekit::bytes (securekit::packet_encryptor::*)()>);
	static_assert(std::is_same_v<decltype(&securekit::packet_encryptor::update),
	    securekit::bytes (securekit::packet_encryptor::*)(std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::packet_encryptor::finalize), securekit::bytes (securekit::packet_encryptor::*)()>);

	static_assert(std::is_same_v<decltype(&securekit::packet_decryptor::begin),
	    void (securekit::packet_decryptor::*)(std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::packet_decryptor::update),
	    securekit::bytes (securekit::packet_decryptor::*)(std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::packet_decryptor::finalize),
	    securekit::bytes (securekit::packet_decryptor::*)(std::span<const std::byte>)>);
}
