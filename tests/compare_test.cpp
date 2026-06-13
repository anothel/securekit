#include "securekit/compare.hpp"
#include "securekit/types.hpp"

#include <cstddef>
#include <initializer_list>

#include <gtest/gtest.h>

namespace
{

securekit::bytes make_bytes(std::initializer_list<unsigned char> values)
{
	securekit::bytes out;
	out.reserve(values.size());
	for (const unsigned char value : values)
	{
		out.push_back(static_cast<std::byte>(value));
	}
	return out;
}

} // namespace

TEST(Compare, EqualInputsReturnTrue)
{
	const securekit::bytes left = make_bytes({0x00, 0x01, 0x80, 0xff});
	const securekit::bytes right = make_bytes({0x00, 0x01, 0x80, 0xff});

	EXPECT_TRUE(securekit::constant_time_equal(left, right));
}

TEST(Compare, SameLengthDifferentInputsReturnFalse)
{
	const securekit::bytes left = make_bytes({0x00, 0x01, 0x80, 0xff});
	const securekit::bytes right = make_bytes({0x00, 0x01, 0x81, 0xff});

	EXPECT_FALSE(securekit::constant_time_equal(left, right));
}

TEST(Compare, DifferentLengthInputsReturnFalse)
{
	const securekit::bytes left = make_bytes({0x00, 0x01, 0x80, 0xff});
	const securekit::bytes right = make_bytes({0x00, 0x01, 0x80});

	EXPECT_FALSE(securekit::constant_time_equal(left, right));
}

TEST(Compare, EmptyInputsReturnTrue)
{
	const securekit::bytes left;
	const securekit::bytes right;

	EXPECT_TRUE(securekit::constant_time_equal(left, right));
}
