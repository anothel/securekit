#include "amv/random.hpp"

#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

TEST(Random, ReturnsRequestedNumberOfBytes)
{
	EXPECT_TRUE(amv::random_bytes(0).empty());
	EXPECT_EQ(amv::random_bytes(1).size(), 1u);
	EXPECT_EQ(amv::random_bytes(16).size(), 16u);
	EXPECT_EQ(amv::random_bytes(1024).size(), 1024u);
}

TEST(Random, GenerateKeyReturnsKey256)
{
	static_assert(std::is_same_v<decltype(amv::generate_key()), amv::key256>);

	const amv::key256 key = amv::generate_key();
	EXPECT_EQ(key.size(), 32u);
}
