#include "securekit/random.hpp"

#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

TEST(Random, ReturnsRequestedNumberOfBytes)
{
	EXPECT_TRUE(securekit::random_bytes(0).empty());
	EXPECT_EQ(securekit::random_bytes(1).size(), 1u);
	EXPECT_EQ(securekit::random_bytes(16).size(), 16u);
	EXPECT_EQ(securekit::random_bytes(1024).size(), 1024u);
}

TEST(Random, GenerateKeyReturnsKey256)
{
	static_assert(std::is_same_v<decltype(securekit::generate_key()), securekit::key256>);

	const securekit::key256 key = securekit::generate_key();
	EXPECT_EQ(key.size(), 32u);
}
