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
	          << "  securekit base64url-encode --text <text>\n"
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

bool is_arg(char *arg, std::string_view expected)
{
	return std::string_view(arg) == expected;
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

		if (argc == 4 && is_arg(argv[1], "base64url-encode") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::base64url_encode(bytes_from_text(argv[3])) << '\n';
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
