#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "fixture_utils.hpp"

namespace
{

std::filesystem::path fixture_dir()
{
	return std::filesystem::path(SECUREKIT_TEST_FIXTURE_DIR);
}

std::filesystem::path format_doc()
{
	return std::filesystem::path(SECUREKIT_FORMAT_DOC);
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
	for (const auto &entry : std::filesystem::recursive_directory_iterator(fixture_dir()))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".hex")
		{
			names.insert(entry.path().lexically_relative(fixture_dir()).generic_string());
		}
	}
	return names;
}

std::set<std::string> actual_negative_hex_fixture_names()
{
	std::set<std::string> names;
	for (const auto &name : actual_hex_fixture_names())
	{
		constexpr std::string_view prefix = "negative/";
		if (name.starts_with(prefix))
		{
			names.insert(name.substr(prefix.size()));
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

std::set<std::string> documented_negative_hex_fixture_names()
{
	const std::string readme = read_text_file(fixture_dir() / "negative" / "README.md");
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
			EXPECT_TRUE(names.insert(token).second) << "duplicate negative fixture README entry: " << token;
		}
		cursor = close + 1;
	}

	return names;
}

std::size_t count_names_with_prefix(const std::set<std::string> &names, std::string_view prefix)
{
	return static_cast<std::size_t>(std::count_if(names.begin(), names.end(), [prefix](const std::string &name) {
		return name.starts_with(prefix);
	}));
}

void expect_fixture_exists(const std::set<std::string> &names, const char *name)
{
	EXPECT_TRUE(names.contains(name)) << "missing required baseline fixture: " << name;
}

void expect_text_contains_terms(std::string_view text, std::string_view description, std::initializer_list<std::string_view> terms)
{
	for (const std::string_view term : terms)
	{
		EXPECT_NE(text.find(term), std::string_view::npos) << description << " missing: " << term;
	}
}

void expect_format_rule_is_mapped(std::string_view format, std::string_view readme, std::string_view format_term, std::string_view readme_term)
{
	EXPECT_NE(format.find(format_term), std::string_view::npos) << "FORMAT.md missing expected rule: " << format_term;
	EXPECT_NE(readme.find(readme_term), std::string_view::npos)
	    << "negative fixture README missing mapped rule: " << readme_term;
}

std::uint32_t read_be32(const securekit::bytes &bytes, std::size_t offset)
{
	return (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[offset])) << 24u) |
	       (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[offset + 1u])) << 16u) |
	       (static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[offset + 2u])) << 8u) |
	       static_cast<std::uint32_t>(std::to_integer<unsigned char>(bytes[offset + 3u]));
}

} // namespace

TEST(FixtureInventory, ReadmeDocumentsEveryHexFixture)
{
	EXPECT_EQ(documented_hex_fixture_names(), actual_hex_fixture_names());
}

TEST(FixtureInventory, NegativeReadmeDocumentsEveryNegativeHexFixture)
{
	EXPECT_EQ(documented_negative_hex_fixture_names(), actual_negative_hex_fixture_names());
}

TEST(FixtureInventory, NegativeReadmeMapsFormatRulesToRegressionTests)
{
	const std::string readme = read_text_file(fixture_dir() / "negative" / "README.md");

	expect_text_contains_terms(readme, "negative compatibility coverage matrix", {
	                                                                                 "## Coverage Matrix",
	                                                                                 "`SKT1` structural format rules",
	                                                                                 "`Aead.RejectsNegativeCompatibilityFixtureMissingTag`",
	                                                                                 "`Aead.RejectsNegativeCompatibilitySkt1HeaderRuleFixtures`",
	                                                                                 "`SKF1` structural format rules",
	                                                                                 "duplicate index is generated in test",
	                                                                                 "`File.RejectsNegativeCompatibilityFixtureNonFinalShortChunk`",
	                                                                                 "`File.RejectsNegativeCompatibilitySkf1FormatRuleFixtures`",
	                                                                                 "`File.DetectsTruncationAppendAndReorder`",
	                                                                                 "`SKP1` structural format rules",
	                                                                                 "skp1-unsupported-scrypt-n.hex",
	                                                                                 "skp1-unsupported-scrypt-r.hex",
	                                                                                 "skp1-unsupported-scrypt-p.hex",
	                                                                                 "`File.PasswordRejectsNegativeCompatibilityFixtureUnsupportedFlags`",
	                                                                                 "`File.PasswordRejectsNegativeCompatibilitySkp1FormatRuleFixtures`",
	                                                                                 "`File.PasswordDetectsTruncationAppendAndReorder`",
	                                                                                 "`File.AuthenticationFailuresUseGenericMessage`",
	                                                                                 "`File.PasswordRejectsWrongPasswordAndAad`",
	                                                                                 "`File.RejectsExistingOutput`",
	                                                                                 "`File.RemovesTemporaryOutputAfterOpenFailure`",
	                                                                                 "`File.RemovesPartialTemporaryOutputAfterLateOpenFailure`",
	                                                                                 "`File.StreamRejectsTrailingDataBeforeWritingFinalPlaintext`",
	                                                                                 "`File.PasswordStreamRejectsTrailingDataBeforeWritingFinalPlaintext`",
	                                                                             });
}

TEST(FixtureInventory, FormatRejectRulesStayMappedToNegativeFixtures)
{
	const std::string format = read_text_file(format_doc());
	const std::string readme = read_text_file(fixture_dir() / "negative" / "README.md");

	expect_format_rule_is_mapped(format, readme, "malformed headers", "malformed magic");
	expect_format_rule_is_mapped(format, readme, "unsupported versions", "unsupported version");
	expect_format_rule_is_mapped(format, readme, "unsupported chunk\nsizes", "unsupported chunk size");
	expect_format_rule_is_mapped(format, readme, "truncated records", "truncated records");
	expect_format_rule_is_mapped(format, readme, "non-monotonic chunk indexes", "non-monotonic");
	expect_format_rule_is_mapped(format, readme, "non-final short chunks", "non-final short chunk");
	expect_format_rule_is_mapped(format, readme, "chunks after the final chunk", "chunk after final");
	expect_format_rule_is_mapped(format, readme, "missing final chunks", "missing final chunk");
	expect_format_rule_is_mapped(format, readme, "unsupported cipher IDs", "unsupported cipher");
	expect_format_rule_is_mapped(format, readme, "unsupported KDF IDs", "unsupported KDF");
	expect_format_rule_is_mapped(format, readme, "unsupported flags", "unsupported flags");
	expect_format_rule_is_mapped(format, readme, "unsupported scrypt parameters", "unsupported scrypt parameters");
}

TEST(FixtureInventory, CoversEverySupportedWireFormatFamily)
{
	const std::set<std::string> names = actual_hex_fixture_names();

	EXPECT_GE(count_names_with_prefix(names, "skt1-aes256-gcm-"), 3u)
	    << "SKT1 AEAD packet fixtures should cover normal, empty, and binary/AAD packets";
	EXPECT_GE(count_names_with_prefix(names, "skt1-key-wrap"), 2u)
	    << "SKT1 key wrap fixtures should cover non-zero and zero wrapped keys";
	EXPECT_GE(count_names_with_prefix(names, "skf1-"), 3u)
	    << "SKF1 file fixtures should cover normal, empty, and binary/AAD files";
	EXPECT_GE(count_names_with_prefix(names, "skp1-"), 3u)
	    << "SKP1 password file fixtures should cover text, empty/AAD, and binary/AAD files";

	expect_fixture_exists(names, "skt1-aes256-gcm-aad.hex");
	expect_fixture_exists(names, "skt1-aes256-gcm-empty.hex");
	expect_fixture_exists(names, "skt1-aes256-gcm-binary-aad.hex");
	expect_fixture_exists(names, "skt1-key-wrap.hex");
	expect_fixture_exists(names, "skt1-key-wrap-zero.hex");
	expect_fixture_exists(names, "skf1-known-file.hex");
	expect_fixture_exists(names, "skf1-empty-aad.hex");
	expect_fixture_exists(names, "skf1-binary-aad.hex");
	expect_fixture_exists(names, "skp1-known-password-file.hex");
	expect_fixture_exists(names, "skp1-empty-aad.hex");
	expect_fixture_exists(names, "skp1-binary-aad.hex");
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

TEST(FixtureInventory, Skt1FixturesUseDocumentedHeader)
{
	const std::set<std::string> names = actual_hex_fixture_names();

	for (const std::string &name : names)
	{
		if (!name.starts_with("skt1-"))
		{
			continue;
		}

		SCOPED_TRACE(name);
		const securekit::bytes fixture = securekit::test::read_hex_fixture(name);
		ASSERT_GE(fixture.size(), 33u);
		EXPECT_EQ(fixture[0], std::byte{'S'});
		EXPECT_EQ(fixture[1], std::byte{'K'});
		EXPECT_EQ(fixture[2], std::byte{'T'});
		EXPECT_EQ(fixture[3], std::byte{'1'});
		EXPECT_EQ(fixture[4], std::byte{0x01});
	}
}

TEST(FixtureInventory, Skf1FixturesUseDocumentedHeader)
{
	const std::set<std::string> names = actual_hex_fixture_names();

	for (const std::string &name : names)
	{
		if (!name.starts_with("skf1-"))
		{
			continue;
		}

		SCOPED_TRACE(name);
		const securekit::bytes fixture = securekit::test::read_hex_fixture(name);
		ASSERT_GE(fixture.size(), 50u + 9u + 16u);
		EXPECT_EQ(fixture[0], std::byte{'S'});
		EXPECT_EQ(fixture[1], std::byte{'K'});
		EXPECT_EQ(fixture[2], std::byte{'F'});
		EXPECT_EQ(fixture[3], std::byte{'1'});
		EXPECT_EQ(fixture[4], std::byte{0x01});
		EXPECT_EQ(fixture[5], std::byte{0x01});
		EXPECT_EQ(read_be32(fixture, 6u), 1048576u);
	}
}

TEST(FixtureInventory, Skp1FixturesUseDocumentedHeader)
{
	const std::set<std::string> names = actual_hex_fixture_names();

	for (const std::string &name : names)
	{
		if (!name.starts_with("skp1-"))
		{
			continue;
		}

		SCOPED_TRACE(name);
		const securekit::bytes fixture = securekit::test::read_hex_fixture(name);
		ASSERT_GE(fixture.size(), 64u + 9u + 16u);
		EXPECT_EQ(fixture[0], std::byte{'S'});
		EXPECT_EQ(fixture[1], std::byte{'K'});
		EXPECT_EQ(fixture[2], std::byte{'P'});
		EXPECT_EQ(fixture[3], std::byte{'1'});
		EXPECT_EQ(fixture[4], std::byte{0x01});
		EXPECT_EQ(fixture[5], std::byte{0x01});
		EXPECT_EQ(fixture[6], std::byte{0x01});
		EXPECT_EQ(fixture[7], std::byte{0x00});
		EXPECT_EQ(read_be32(fixture, 8u), 1048576u);
		EXPECT_EQ(read_be32(fixture, 52u), 32768u);
		EXPECT_EQ(read_be32(fixture, 56u), 8u);
		EXPECT_EQ(read_be32(fixture, 60u), 1u);
	}
}
