#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

std::filesystem::path fixture_dir()
{
	return std::filesystem::path(SECUREKIT_TEST_FIXTURE_DIR);
}

std::string read_text_file(const std::filesystem::path &path)
{
	std::ifstream in(path);
	if (!in)
	{
		throw std::runtime_error("failed to open file: " + path.string());
	}
	return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

bool is_ascii_whitespace(unsigned char ch)
{
	return std::isspace(ch) != 0;
}

bool is_lowercase_hex_digit(unsigned char ch)
{
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
}

std::string without_ascii_whitespace(std::string text)
{
	text.erase(
	    std::remove_if(text.begin(), text.end(), [](unsigned char ch) { return is_ascii_whitespace(ch); }),
	    text.end());
	return text;
}

std::set<std::string> actual_hex_fixture_names()
{
	std::set<std::string> names;
	for (const auto &entry : std::filesystem::directory_iterator(fixture_dir()))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".hex")
		{
			names.insert(entry.path().filename().string());
		}
	}
	return names;
}

std::set<std::string> documented_hex_fixture_names()
{
	const std::string readme = read_text_file(fixture_dir() / "README.md");
	std::set<std::string> names;
	std::size_t cursor = 0;

	while ((cursor = readme.find('`', cursor)) != std::string::npos)
	{
		const std::size_t close = readme.find('`', cursor + 1);
		if (close == std::string::npos)
		{
			break;
		}

		const std::string token = readme.substr(cursor + 1, close - cursor - 1);
		if (token.ends_with(".hex"))
		{
			EXPECT_TRUE(names.insert(token).second) << "duplicate fixture README entry: " << token;
		}
		cursor = close + 1;
	}

	return names;
}

} // namespace

TEST(FixtureInventory, ReadmeDocumentsEveryHexFixture)
{
	EXPECT_EQ(documented_hex_fixture_names(), actual_hex_fixture_names());
}

TEST(FixtureInventory, HexFixturesAreCanonical)
{
	for (const std::string &name : actual_hex_fixture_names())
	{
		const std::string text = read_text_file(fixture_dir() / name);
		const std::string hex = without_ascii_whitespace(text);

		EXPECT_FALSE(hex.empty()) << name;
		EXPECT_EQ(hex.size() % 2, 0u) << name;
		for (const unsigned char ch : text)
		{
			EXPECT_TRUE(is_ascii_whitespace(ch) || is_lowercase_hex_digit(ch))
			    << name << " contains non-canonical hex character";
		}
	}
}
