#include "securekit/aead.hpp"
#include "securekit/base64.hpp"
#include "securekit/compare.hpp"
#include "securekit/error.hpp"
#include "securekit/file.hpp"
#include "securekit/hash.hpp"
#include "securekit/hex.hpp"
#include "securekit/key_wrap.hpp"
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
}

TEST(PublicHeaders, KeyWrapApiIsAvailable)
{
	static_assert(std::is_same_v<decltype(&securekit::wrap_key), securekit::bytes (*)(const securekit::key256 &, const securekit::key256 &, std::span<const std::byte>)>);
	static_assert(std::is_same_v<decltype(&securekit::unwrap_key), securekit::key256 (*)(std::span<const std::byte>, const securekit::key256 &, std::span<const std::byte>)>);
}
