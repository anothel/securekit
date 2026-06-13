#include "securekit/hex.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

#include "securekit/error.hpp"

namespace
{

securekit::bytes make_bytes(std::initializer_list<unsigned int> values)
{
	securekit::bytes result;
	result.reserve(values.size());
	for (unsigned int value : values)
	{
		result.push_back(static_cast<std::byte>(value));
	}
	return result;
}

} // namespace

TEST(Hex, EncodesLowercaseWithoutSeparators)
{
	const auto input = make_bytes({0x00, 0x01, 0x0f, 0x10, 0xab, 0xff});
	EXPECT_EQ(securekit::hex_encode(input), "00010f10abff");
}

TEST(Hex, EncodesEmptyInput)
{
	const securekit::bytes input;
	EXPECT_EQ(securekit::hex_encode(input), "");
}

TEST(Hex, DecodesUppercaseAndLowercase)
{
	const auto decoded = securekit::hex_decode("00010F10abFF");
	EXPECT_EQ(decoded, make_bytes({0x00, 0x01, 0x0f, 0x10, 0xab, 0xff}));
}

TEST(Hex, DecodesEmptyInput)
{
	EXPECT_TRUE(securekit::hex_decode("").empty());
}

TEST(Hex, RejectsOddLength)
{
	try
	{
		(void)securekit::hex_decode("abc");
		FAIL() << "expected securekit::error";
	}
	catch (const securekit::error &e)
	{
		EXPECT_EQ(e.code(), securekit::error_code::invalid_encoding);
	}
}

TEST(Hex, RejectsInvalidCharactersAndSeparators)
{
	for (std::string_view input : {"00 01", "0x00", "00:01", "00-01", "00zz"})
	{
		try
		{
			(void)securekit::hex_decode(input);
			FAIL() << "expected securekit::error for " << input;
		}
		catch (const securekit::error &e)
		{
			EXPECT_EQ(e.code(), securekit::error_code::invalid_encoding);
		}
	}
}
