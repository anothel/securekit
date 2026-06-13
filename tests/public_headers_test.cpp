#include "amv/aead.hpp"
#include "amv/amv.hpp"
#include "amv/base64.hpp"
#include "amv/error.hpp"
#include "amv/hash.hpp"
#include "amv/hex.hpp"
#include "amv/random.hpp"
#include "amv/types.hpp"

#include <gtest/gtest.h>

TEST(PublicHeaders, TypeAliasesAreAvailable)
{
	amv::bytes data;
	amv::key256 key{};
	amv::digest256 digest{};

	EXPECT_TRUE(data.empty());
	EXPECT_EQ(key.size(), 32u);
	EXPECT_EQ(digest.size(), 32u);
}
