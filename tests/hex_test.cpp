#include "amv/hex.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

#include "amv/error.hpp"

namespace
{

amv::bytes make_bytes(std::initializer_list<unsigned int> values)
{
	amv::bytes result;
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
	EXPECT_EQ(amv::hex_encode(input), "00010f10abff");
}

TEST(Hex, EncodesEmptyInput)
{
	const amv::bytes input;
	EXPECT_EQ(amv::hex_encode(input), "");
}

TEST(Hex, DecodesUppercaseAndLowercase)
{
	const auto decoded = amv::hex_decode("00010F10abFF");
	EXPECT_EQ(decoded, make_bytes({0x00, 0x01, 0x0f, 0x10, 0xab, 0xff}));
}

TEST(Hex, DecodesEmptyInput)
{
	EXPECT_TRUE(amv::hex_decode("").empty());
}

TEST(Hex, RejectsOddLength)
{
	try
	{
		(void)amv::hex_decode("abc");
		FAIL() << "expected amv::error";
	}
	catch (const amv::error &e)
	{
		EXPECT_EQ(e.code(), amv::error_code::invalid_encoding);
	}
}

TEST(Hex, RejectsInvalidCharactersAndSeparators)
{
	for (std::string_view input : {"00 01", "0x00", "00:01", "00-01", "00zz"})
	{
		try
		{
			(void)amv::hex_decode(input);
			FAIL() << "expected amv::error for " << input;
		}
		catch (const amv::error &e)
		{
			EXPECT_EQ(e.code(), amv::error_code::invalid_encoding);
		}
	}
}
