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
	          << "  securekit seal-file --in <path> --out <path> --key-hex <64-hex> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit open-file --in <path> --out <path> --key-hex <64-hex> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit seal-file --in <path> --out <path> --key-file <path> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit open-file --in <path> --out <path> --key-file <path> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit help [command]\n";
}

std::string_view token_usage()
{
	return "Usage:\n  securekit token <byte-size>";
}

std::string_view sha256_usage()
{
	return "Usage:\n  securekit sha256 --text <text>\n  securekit sha256 --file <path>";
}

std::string_view hex_encode_usage()
{
	return "Usage:\n  securekit hex-encode --text <text>";
}

std::string_view hex_decode_usage()
{
	return "Usage:\n  securekit hex-decode --text <hex>";
}

std::string_view base64url_encode_usage()
{
	return "Usage:\n  securekit base64url-encode --text <text>";
}

std::string_view base64url_decode_usage()
{
	return "Usage:\n  securekit base64url-decode --text <base64url>";
}

std::string_view keygen_usage()
{
	return "Usage:\n  securekit keygen --out <path>";
}

std::string_view file_command_usage(std::string_view command)
{
	if (command == "seal-file")
	{
		return "Usage:\n  securekit seal-file --in <path> --out <path> (--key-hex <64-hex>|--key-file <path>) "
		       "[--aad-text <text>|--aad-hex <hex>]";
	}
	return "Usage:\n  securekit open-file --in <path> --out <path> (--key-hex <64-hex>|--key-file <path>) "
	       "[--aad-text <text>|--aad-hex <hex>]";
}

bool print_command_help(std::string_view command)
{
	if (command == "token")
	{
		std::cout << token_usage() << '\n';
		return true;
	}
	if (command == "sha256")
	{
		std::cout << sha256_usage() << '\n';
		return true;
	}
	if (command == "hex-encode")
	{
		std::cout << hex_encode_usage() << '\n';
		return true;
	}
	if (command == "hex-decode")
	{
		std::cout << hex_decode_usage() << '\n';
		return true;
	}
	if (command == "base64url-encode")
	{
		std::cout << base64url_encode_usage() << '\n';
		return true;
	}
	if (command == "base64url-decode")
	{
		std::cout << base64url_decode_usage() << '\n';
		return true;
	}
	if (command == "keygen")
	{
		std::cout << keygen_usage() << '\n';
		return true;
	}
	if (command == "seal-file")
	{
		std::cout << file_command_usage(command) << '\n';
		return true;
	}
	if (command == "open-file")
	{
		std::cout << file_command_usage(command) << '\n';
		return true;
	}
	return false;
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

securekit::key256 key_from_option(std::string_view option, std::string_view value)
{
	if (option == "--key-hex")
	{
		return key_from_hex(value);
	}
	if (option == "--key-file")
	{
		return key_from_file(std::filesystem::path(value));
	}

	throw std::runtime_error("unsupported command");
}

securekit::bytes aad_from_option(std::string_view option, std::string_view value)
{
	if (option == "--aad-text")
	{
		return bytes_from_text(value);
	}
	if (option == "--aad-hex")
	{
		return securekit::hex_decode(value);
	}

	throw std::runtime_error("unsupported command");
}

struct file_command_options
{
	std::filesystem::path input;
	std::filesystem::path output;
	securekit::key256 key{};
	securekit::bytes aad;
	bool has_input = false;
	bool has_output = false;
	bool has_key = false;
	bool has_aad = false;
};

void reject_duplicate(bool already_set, std::string_view option)
{
	if (already_set)
	{
		throw std::runtime_error(std::string("duplicate option: ") + std::string(option));
	}
}

file_command_options parse_file_command_options(int argc, char **argv)
{
	if (argc < 8 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(file_command_usage(argv[1])));
	}

	file_command_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--in")
		{
			reject_duplicate(options.has_input, option);
			options.input = std::filesystem::path(value);
			options.has_input = true;
		}
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.has_output = true;
		}
		else if (option == "--key-hex" || option == "--key-file")
		{
			if (options.has_key)
			{
				throw std::runtime_error("conflicting key options");
			}
			options.key = key_from_option(option, value);
			options.has_key = true;
		}
		else if (option == "--aad-text" || option == "--aad-hex")
		{
			if (options.has_aad)
			{
				throw std::runtime_error("conflicting AAD options");
			}
			options.aad = aad_from_option(option, value);
			options.has_aad = true;
		}
		else
		{
			throw std::runtime_error(std::string("unsupported file option: ") + std::string(option));
		}
	}

	if (!options.has_input || !options.has_output || !options.has_key)
	{
		throw std::runtime_error(std::string(file_command_usage(argv[1])));
	}

	return options;
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
		if (argc == 1)
		{
			print_help();
			return 0;
		}

		if (argc == 2 && (std::string_view(argv[1]) == "help" || std::string_view(argv[1]) == "--help"))
		{
			print_help();
			return 0;
		}

		if (argc == 3 && is_arg(argv[1], "help"))
		{
			if (print_command_help(argv[2]))
			{
				return 0;
			}
			return fail("unsupported command");
		}

		if (argc == 3 && is_arg(argv[1], "token"))
		{
			std::cout << securekit::random_token(parse_size(argv[2])) << '\n';
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "token"))
		{
			return fail(token_usage());
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

		if (argc >= 2 && is_arg(argv[1], "sha256"))
		{
			return fail(sha256_usage());
		}

		if (argc == 4 && is_arg(argv[1], "hex-encode") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::hex_encode(bytes_from_text(argv[3])) << '\n';
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "hex-encode"))
		{
			return fail(hex_encode_usage());
		}

		if (argc == 4 && is_arg(argv[1], "hex-decode") && is_arg(argv[2], "--text"))
		{
			write_bytes(securekit::hex_decode(argv[3]));
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "hex-decode"))
		{
			return fail(hex_decode_usage());
		}

		if (argc == 4 && is_arg(argv[1], "base64url-encode") && is_arg(argv[2], "--text"))
		{
			std::cout << securekit::base64url_encode(bytes_from_text(argv[3])) << '\n';
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "base64url-encode"))
		{
			return fail(base64url_encode_usage());
		}

		if (argc == 4 && is_arg(argv[1], "base64url-decode") && is_arg(argv[2], "--text"))
		{
			write_bytes(securekit::base64url_decode(argv[3]));
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "base64url-decode"))
		{
			return fail(base64url_decode_usage());
		}

		if (argc == 4 && is_arg(argv[1], "keygen") && is_arg(argv[2], "--out"))
		{
			write_generated_key(argv[3]);
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "keygen"))
		{
			return fail(keygen_usage());
		}

		if (argc >= 2 && (is_arg(argv[1], "seal-file") || is_arg(argv[1], "open-file")))
		{
			const file_command_options options = parse_file_command_options(argc, argv);

			if (is_arg(argv[1], "seal-file"))
			{
				securekit::seal_file(options.input, options.output, options.key, options.aad);
			}
			else
			{
				securekit::open_file(options.input, options.output, options.key, options.aad);
			}
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
