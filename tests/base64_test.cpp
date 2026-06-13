#include "securekit/base64.hpp"

#include <array>
#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "securekit/error.hpp"

namespace
{

securekit::bytes bytes_from_ascii(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (char ch : text)
	{
		out.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
	}
	return out;
}

} // namespace

TEST(Base64, EncodesRfc4648Vectors)
{
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("")), "");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("f")), "Zg==");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("fo")), "Zm8=");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("foo")), "Zm9v");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("foob")), "Zm9vYg==");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("fooba")), "Zm9vYmE=");
	EXPECT_EQ(securekit::base64_encode(bytes_from_ascii("foobar")), "Zm9vYmFy");
}

TEST(Base64, DecodesRfc4648Vectors)
{
	EXPECT_EQ(securekit::base64_decode(""), bytes_from_ascii(""));
	EXPECT_EQ(securekit::base64_decode("Zg=="), bytes_from_ascii("f"));
	EXPECT_EQ(securekit::base64_decode("Zm8="), bytes_from_ascii("fo"));
	EXPECT_EQ(securekit::base64_decode("Zm9v"), bytes_from_ascii("foo"));
	EXPECT_EQ(securekit::base64_decode("Zm9vYg=="), bytes_from_ascii("foob"));
	EXPECT_EQ(securekit::base64_decode("Zm9vYmE="), bytes_from_ascii("fooba"));
	EXPECT_EQ(securekit::base64_decode("Zm9vYmFy"), bytes_from_ascii("foobar"));
}

TEST(Base64, EncodesBase64UrlWithoutPadding)
{
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("")), "");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("f")), "Zg");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("fo")), "Zm8");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("foo")), "Zm9v");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("foob")), "Zm9vYg");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("fooba")), "Zm9vYmE");
	EXPECT_EQ(securekit::base64url_encode(bytes_from_ascii("foobar")), "Zm9vYmFy");
}

TEST(Base64, DecodesBase64UrlWithoutPadding)
{
	EXPECT_EQ(securekit::base64url_decode(""), bytes_from_ascii(""));
	EXPECT_EQ(securekit::base64url_decode("Zg"), bytes_from_ascii("f"));
	EXPECT_EQ(securekit::base64url_decode("Zm8"), bytes_from_ascii("fo"));
	EXPECT_EQ(securekit::base64url_decode("Zm9v"), bytes_from_ascii("foo"));
	EXPECT_EQ(securekit::base64url_decode("Zm9vYg"), bytes_from_ascii("foob"));
	EXPECT_EQ(securekit::base64url_decode("Zm9vYmE"), bytes_from_ascii("fooba"));
	EXPECT_EQ(securekit::base64url_decode("Zm9vYmFy"), bytes_from_ascii("foobar"));
}

TEST(Base64, Base64UrlUsesUrlSafeAlphabet)
{
	const securekit::bytes input{std::byte{0xfb}, std::byte{0xff}, std::byte{0xff}};

	EXPECT_EQ(securekit::base64url_encode(input), "-___");
	EXPECT_EQ(securekit::base64url_decode("-___"), input);
}

TEST(Base64, RejectsMalformedBase64UrlInput)
{
	constexpr std::array<std::string_view, 8> inputs{
	    "Z",
	    "Zg=",
	    "Zg==",
	    "Z g",
	    "Zm9v+",
	    "Zm9v/",
	    "Zh",
	    "Zm9",
	};

	for (std::string_view input : inputs)
	{
		try
		{
			(void)securekit::base64url_decode(input);
			FAIL() << "expected securekit::error for " << input;
		}
		catch (const securekit::error &e)
		{
			EXPECT_EQ(e.code(), securekit::error_code::invalid_encoding);
		}
	}
}

TEST(Base64, RejectsMalformedInput)
{
	constexpr std::array<std::string_view, 10> inputs{
	    "Zg",
	    "Zg=",
	    "Zg===",
	    "Z g==",
	    "Zg==x",
	    "Zm9v-",
	    "Zm9v_",
	    "====",
	    "Zh==",
	    "Zm9=",
	};

	for (std::string_view input : inputs)
	{
		try
		{
			(void)securekit::base64_decode(input);
			FAIL() << "expected securekit::error for " << input;
		}
		catch (const securekit::error &e)
		{
			EXPECT_EQ(e.code(), securekit::error_code::invalid_encoding);
		}
	}
}
