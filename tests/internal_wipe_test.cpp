#include "../src/wipe.hpp"

#include <algorithm>
#include <cstddef>

#include <gtest/gtest.h>

TEST(InternalWipe, OverwritesByteBuffer)
{
	securekit::bytes data{std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};

	securekit::internal::secure_wipe(data);

	EXPECT_TRUE(std::all_of(data.begin(), data.end(), [](std::byte value) { return value == std::byte{0}; }));
}
