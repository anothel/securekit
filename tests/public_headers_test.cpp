#include "securekit/aead.hpp"
#include "securekit/base64.hpp"
#include "securekit/compare.hpp"
#include "securekit/error.hpp"
#include "securekit/hash.hpp"
#include "securekit/hex.hpp"
#include "securekit/random.hpp"
#include "securekit/securekit.hpp"
#include "securekit/types.hpp"

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
