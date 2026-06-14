#include "securekit/securekit.hpp"

#include <charconv>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace
{

void print_help()
{
	std::cout << "Usage:\n"
	          << "  securekit token <byte-size>\n"
	          << "  securekit sha256 --text <text>\n"
	          << "  securekit sha256 --file <path>\n"
	          << "  securekit hex-encode --text <text>\n"
	          << "  securekit hex-decode --text <hex>\n"
	          << "  securekit base64url-encode --text <text>\n"
	          << "  securekit base64url-decode --text <base64url>\n"
	          << "  securekit keygen --out <path>\n"
	          << "  securekit seal-file --in <path> --out <path> --key-hex <64-hex>\n"
	          << "  securekit open-file --in <path> --out <path> --key-hex <64-hex>\n"
	          << "  securekit seal-file --in <path> --out <path> --key-file <path>\n"
	          << "  securekit open-file --in <path> --out <path> --key-file <path>\n"
	          << "  securekit help\n";
}

int fail(std::string_view message)
{
	std::cerr << message << '\n';
	return 1;
}

securekit::bytes bytes_from_text(std::string_view text)
{
	securekit::bytes bytes;
	bytes.reserve(text.size());
	for (const unsigned char ch : text)
	{
		bytes.push_back(static_cast<std::byte>(ch));
	}
	return bytes;
}

void write_bytes(const securekit::bytes &bytes)
{
	for (const std::byte byte : bytes)
	{
		std::cout.put(static_cast<char>(std::to_integer<unsigned char>(byte)));
	}
	std::cout << '\n';
}

bool is_arg(char *arg, std::string_view expected)
{
	return std::string_view(arg) == expected;
}

bool is_hex_char(char ch)
{
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

securekit::key256 key_from_hex(std::string_view text)
{
	if (text.size() != 64)
	{
		throw std::runtime_error("key must be 64 hex characters");
	}
	for (const char ch : text)
	{
		if (!is_hex_char(ch))
		{
			throw std::runtime_error("key must be 64 hex characters");
		}
	}

	const securekit::bytes decoded = securekit::hex_decode(text);
	securekit::key256 key{};
	for (std::size_t index = 0; index < key.size(); ++index)
	{
		key[index] = decoded[index];
	}
	return key;
}

std::string trim_ascii_whitespace(std::string_view text)
{
	const auto is_space = [](char ch) {
		return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
	};

	std::size_t first = 0;
	while (first < text.size() && is_space(text[first]))
	{
		++first;
	}

	std::size_t last = text.size();
	while (last > first && is_space(text[last - 1]))
	{
		--last;
	}

	return std::string(text.substr(first, last - first));
}

securekit::bytes read_file(const std::filesystem::path &path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
	{
		throw std::runtime_error("failed to open input file");
	}

	const std::string data{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	if (!in.good() && !in.eof())
	{
		throw std::runtime_error("failed to read input file");
	}

	securekit::bytes bytes;
	bytes.reserve(data.size());
	for (const unsigned char ch : data)
	{
		bytes.push_back(static_cast<std::byte>(ch));
	}
	return bytes;
}

securekit::key256 key_from_file(const std::filesystem::path &path)
{
	const securekit::bytes key_bytes = read_file(path);
	std::string text;
	text.reserve(key_bytes.size());
	for (const std::byte byte : key_bytes)
	{
		text.push_back(static_cast<char>(std::to_integer<unsigned char>(byte)));
	}
	return key_from_hex(trim_ascii_whitespace(text));
}

void write_generated_key(const std::filesystem::path &path)
{
	if (std::filesystem::exists(path))
	{
		throw std::runtime_error("Output file already exists");
	}

	std::ofstream out(path, std::ios::binary);
	if (!out)
	{
		throw std::runtime_error("failed to open output file");
	}

	out << securekit::hex_encode(securekit::generate_key()) << '\n';
	if (!out)
	{
		throw std::runtime_error("failed to write output file");
	}
}

std::size_t parse_size(std::string_view text)
{
	if (text.empty())
	{
		throw std::runtime_error("byte-size must be a positive decimal integer");
	}

	std::size_t value = 0;
	const char *first = text.data();
	const char *last = text.data() + text.size();
	const auto result = std::from_chars(first, last, value);
	if (result.ec != std::errc{} || result.ptr != last || value == 0)
	{
		throw std::runtime_error("byte-size must be a positive decimal integer");
	}
	return value;
}

} // namespace

int main(int argc, char **argv)
{
	try
	{
		if (argc == 2 && (std::string_view(argv[1]) == "help" || std::string_view(argv[1]) == "--help"))
		{
			print_help();
			return 0;
		}

		if (argc == 3 && is_arg(argv[1], "token"))
		{
			std::cout << securekit::random_token(parse_size(argv[2])) << '\n';
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "sha256") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::hex_encode(securekit::sha256(bytes_from_text(argv[3]))) << '\n';
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "sha256") && is_arg(argv[2], "--file"))
		{
			std::cout << securekit::hex_encode(securekit::sha256(read_file(argv[3]))) << '\n';
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "hex-encode") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::hex_encode(bytes_from_text(argv[3])) << '\n';
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "hex-decode") && is_arg(argv[2], "--text"))
		{
			write_bytes(securekit::hex_decode(argv[3]));
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "base64url-encode") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::base64url_encode(bytes_from_text(argv[3])) << '\n';
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "base64url-decode") && is_arg(argv[2], "--text"))
		{
			write_bytes(securekit::base64url_decode(argv[3]));
			return 0;
		}

		if (argc == 4 && is_arg(argv[1], "keygen") && is_arg(argv[2], "--out"))
		{
			write_generated_key(argv[3]);
			return 0;
		}

		if (argc == 8 && is_arg(argv[1], "seal-file") && is_arg(argv[2], "--in") && is_arg(argv[4], "--out") &&
		    is_arg(argv[6], "--key-hex"))
		{
			securekit::seal_file(argv[3], argv[5], key_from_hex(argv[7]));
			return 0;
		}

		if (argc == 8 && is_arg(argv[1], "open-file") && is_arg(argv[2], "--in") && is_arg(argv[4], "--out") &&
		    is_arg(argv[6], "--key-hex"))
		{
			securekit::open_file(argv[3], argv[5], key_from_hex(argv[7]));
			return 0;
		}

		if (argc == 8 && is_arg(argv[1], "seal-file") && is_arg(argv[2], "--in") && is_arg(argv[4], "--out") &&
		    is_arg(argv[6], "--key-file"))
		{
			securekit::seal_file(argv[3], argv[5], key_from_file(argv[7]));
			return 0;
		}

		if (argc == 8 && is_arg(argv[1], "open-file") && is_arg(argv[2], "--in") && is_arg(argv[4], "--out") &&
		    is_arg(argv[6], "--key-file"))
		{
			securekit::open_file(argv[3], argv[5], key_from_file(argv[7]));
			return 0;
		}

		return fail("unsupported command");
	}
	catch (const securekit::error &e)
	{
		return fail(e.what());
	}
	catch (const std::exception &e)
	{
		return fail(e.what());
	}
}
