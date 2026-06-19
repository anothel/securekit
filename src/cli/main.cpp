#include "securekit/securekit.hpp"

#include <charconv>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace
{

void print_help()
{
	std::cout << "Usage:\n"
	          << "  securekit token <byte-size>\n"
	          << "  securekit sha256 --text <text>\n"
	          << "  securekit sha256 --file <path>\n"
	          << "  securekit hmac-sha256 --key-hex <hex> --text <text>\n"
	          << "  securekit hmac-sha256 --key-hex <hex> --file <path>\n"
	          << "  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-hex <hex> --out-size "
	             "<byte-size>\n"
	          << "  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-text <text> --out-size "
	             "<byte-size>\n"
	          << "  securekit hex-encode --text <text>\n"
	          << "  securekit hex-decode --text <hex>\n"
	          << "  securekit base64url-encode --text <text>\n"
	          << "  securekit base64url-decode --text <base64url>\n"
	          << "  securekit keygen --out <path>\n"
	          << "  securekit wrap-key (--key-hex <64-hex>|--key-file <path>) "
	             "(--wrapping-key-hex <64-hex>|--wrapping-key-file <path>) [--out <path>]\n"
	          << "  securekit unwrap-key (--packet-hex <hex>|--packet-file <path>) "
	             "(--wrapping-key-hex <64-hex>|--wrapping-key-file <path>) [--out <path>]\n"
	          << "  securekit encrypt (--text <text>|--in <path>) (--key-hex <64-hex>|--key-file <path>) "
	             "[--aad-text <text>|--aad-hex <hex>] [--out <path>]\n"
	          << "  securekit decrypt (--packet-hex <hex>|--packet-file <path>) "
	             "(--key-hex <64-hex>|--key-file <path>) [--aad-text <text>|--aad-hex <hex>] [--out <path>]\n"
	          << "  securekit seal-file --in <path|-> --out <path|-> --key-hex <64-hex> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit open-file --in <path|-> --out <path|-> --key-hex <64-hex> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit seal-file --in <path|-> --out <path|-> --key-file <path> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit open-file --in <path|-> --out <path|-> --key-file <path> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit seal-file-password --in <path|-> --out <path|-> --password-file <path> "
	             "[--aad-text <text>|--aad-hex <hex>]\n"
	          << "  securekit open-file-password --in <path|-> --out <path|-> --password-file <path> "
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

std::string_view hmac_sha256_usage()
{
	return "Usage:\n  securekit hmac-sha256 --key-hex <hex> --text <text>\n  securekit hmac-sha256 --key-hex "
	       "<hex> --file <path>";
}

std::string_view hkdf_sha256_usage()
{
	return "Usage:\n  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-hex <hex> --out-size "
	       "<byte-size>\n  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-text <text> --out-size "
	       "<byte-size>";
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

std::string_view wrap_key_usage()
{
	return "Usage:\n  securekit wrap-key (--key-hex <64-hex>|--key-file <path>) "
	       "(--wrapping-key-hex <64-hex>|--wrapping-key-file <path>) [--out <path>]";
}

std::string_view unwrap_key_usage()
{
	return "Usage:\n  securekit unwrap-key (--packet-hex <hex>|--packet-file <path>) "
	       "(--wrapping-key-hex <64-hex>|--wrapping-key-file <path>) [--out <path>]";
}

std::string_view packet_command_usage(std::string_view command)
{
	if (command == "encrypt")
	{
		return "Usage:\n  securekit encrypt (--text <text>|--in <path>) (--key-hex <64-hex>|--key-file <path>) "
		       "[--aad-text <text>|--aad-hex <hex>] [--out <path>]";
	}
	return "Usage:\n  securekit decrypt (--packet-hex <hex>|--packet-file <path>) "
	       "(--key-hex <64-hex>|--key-file <path>) [--aad-text <text>|--aad-hex <hex>] [--out <path>]";
}

std::string_view file_command_usage(std::string_view command)
{
	if (command == "seal-file")
	{
		return "Usage:\n  securekit seal-file --in <path|-> --out <path|-> (--key-hex <64-hex>|--key-file <path>) "
		       "[--aad-text <text>|--aad-hex <hex>]";
	}
	return "Usage:\n  securekit open-file --in <path|-> --out <path|-> (--key-hex <64-hex>|--key-file <path>) "
	       "[--aad-text <text>|--aad-hex <hex>]";
}

std::string_view password_file_command_usage(std::string_view command)
{
	if (command == "seal-file-password")
	{
		return "Usage:\n  securekit seal-file-password --in <path|-> --out <path|-> --password-file <path> "
		       "[--aad-text <text>|--aad-hex <hex>]";
	}
	return "Usage:\n  securekit open-file-password --in <path|-> --out <path|-> --password-file <path> "
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
	if (command == "hmac-sha256")
	{
		std::cout << hmac_sha256_usage() << '\n';
		return true;
	}
	if (command == "hkdf-sha256")
	{
		std::cout << hkdf_sha256_usage() << '\n';
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
	if (command == "wrap-key")
	{
		std::cout << wrap_key_usage() << '\n';
		return true;
	}
	if (command == "unwrap-key")
	{
		std::cout << unwrap_key_usage() << '\n';
		return true;
	}
	if (command == "encrypt")
	{
		std::cout << packet_command_usage(command) << '\n';
		return true;
	}
	if (command == "decrypt")
	{
		std::cout << packet_command_usage(command) << '\n';
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
	if (command == "seal-file-password")
	{
		std::cout << password_file_command_usage(command) << '\n';
		return true;
	}
	if (command == "open-file-password")
	{
		std::cout << password_file_command_usage(command) << '\n';
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

securekit::bytes password_from_file(const std::filesystem::path &path)
{
	securekit::bytes password = read_file(path);
	if (password.empty())
	{
		throw std::runtime_error("password must not be empty");
	}
	return password;
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

securekit::key256 wrapping_key_from_option(std::string_view option, std::string_view value)
{
	if (option == "--wrapping-key-hex")
	{
		return key_from_hex(value);
	}
	if (option == "--wrapping-key-file")
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
	bool input_is_stdin = false;
	bool output_is_stdout = false;
};

struct password_file_command_options
{
	std::filesystem::path input;
	std::filesystem::path output;
	securekit::bytes password;
	securekit::bytes aad;
	bool has_input = false;
	bool has_output = false;
	bool has_password = false;
	bool has_aad = false;
	bool input_is_stdin = false;
	bool output_is_stdout = false;
};

struct wrap_key_options
{
	securekit::key256 key{};
	securekit::key256 wrapping_key{};
	std::filesystem::path output;
	bool has_key = false;
	bool has_wrapping_key = false;
	bool has_output = false;
};

struct unwrap_key_options
{
	securekit::bytes packet;
	securekit::key256 wrapping_key{};
	std::filesystem::path output;
	bool has_packet = false;
	bool has_wrapping_key = false;
	bool has_output = false;
};

struct encrypt_options
{
	securekit::bytes plaintext;
	securekit::key256 key{};
	securekit::bytes aad;
	std::filesystem::path output;
	bool has_plaintext = false;
	bool has_key = false;
	bool has_aad = false;
	bool has_output = false;
};

struct decrypt_options
{
	securekit::bytes packet;
	securekit::key256 key{};
	securekit::bytes aad;
	std::filesystem::path output;
	bool has_packet = false;
	bool has_key = false;
	bool has_aad = false;
	bool has_output = false;
};

void reject_duplicate(bool already_set, std::string_view option)
{
	if (already_set)
	{
		throw std::runtime_error(std::string("duplicate option: ") + std::string(option));
	}
}

bool is_standard_stream_marker(std::string_view value)
{
	return value == "-";
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
			options.input_is_stdin = is_standard_stream_marker(value);
			options.has_input = true;
		}
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.output_is_stdout = is_standard_stream_marker(value);
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

password_file_command_options parse_password_file_command_options(int argc, char **argv)
{
	if (argc < 8 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(password_file_command_usage(argv[1])));
	}

	password_file_command_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--in")
		{
			reject_duplicate(options.has_input, option);
			options.input = std::filesystem::path(value);
			options.input_is_stdin = is_standard_stream_marker(value);
			options.has_input = true;
		}
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.output_is_stdout = is_standard_stream_marker(value);
			options.has_output = true;
		}
		else if (option == "--password-file")
		{
			reject_duplicate(options.has_password, option);
			options.password = password_from_file(std::filesystem::path(value));
			options.has_password = true;
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
			throw std::runtime_error(std::string("unsupported password file option: ") + std::string(option));
		}
	}

	if (!options.has_input || !options.has_output || !options.has_password)
	{
		throw std::runtime_error(std::string(password_file_command_usage(argv[1])));
	}

	return options;
}

void ensure_output_file_does_not_exist(const std::filesystem::path &path)
{
	if (std::filesystem::exists(path))
	{
		throw std::runtime_error("Output file already exists");
	}
}

void write_binary_file(const std::filesystem::path &path, std::span<const std::byte> bytes)
{
	ensure_output_file_does_not_exist(path);

	std::ofstream out(path, std::ios::binary);
	if (!out)
	{
		throw std::runtime_error("failed to open output file");
	}

	out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	if (!out)
	{
		throw std::runtime_error("failed to write output file");
	}
}

void write_key_file(const std::filesystem::path &path, const securekit::key256 &key)
{
	ensure_output_file_does_not_exist(path);

	std::ofstream out(path, std::ios::binary);
	if (!out)
	{
		throw std::runtime_error("failed to open output file");
	}

	out << securekit::hex_encode(key) << '\n';
	if (!out)
	{
		throw std::runtime_error("failed to write output file");
	}
}

void write_generated_key(const std::filesystem::path &path)
{
	write_key_file(path, securekit::generate_key());
}

void set_standard_stream_binary(int descriptor)
{
#ifdef _WIN32
	if (_setmode(descriptor, _O_BINARY) == -1)
	{
		throw std::runtime_error("failed to set standard stream binary mode");
	}
#else
	(void)descriptor;
#endif
}

void set_stdin_binary()
{
#ifdef _WIN32
	set_standard_stream_binary(_fileno(stdin));
#else
	set_standard_stream_binary(0);
#endif
}

void set_stdout_binary()
{
#ifdef _WIN32
	set_standard_stream_binary(_fileno(stdout));
#else
	set_standard_stream_binary(1);
#endif
}

std::ifstream open_cli_input_file(const std::filesystem::path &path)
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		throw std::runtime_error("failed to open input file");
	}
	return input;
}

std::ofstream open_cli_output_file(const std::filesystem::path &path)
{
	std::ofstream output(path, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		throw std::runtime_error("failed to open output file");
	}
	return output;
}

std::filesystem::path temporary_output_path_for(const std::filesystem::path &output)
{
	std::filesystem::path temp_path = output;
	temp_path += ".securekit.tmp";
	return temp_path;
}

void remove_file_quietly(const std::filesystem::path &path)
{
	std::error_code ec;
	(void)std::filesystem::remove(path, ec);
}

void rename_output_file(const std::filesystem::path &temp_path, const std::filesystem::path &output)
{
	std::error_code ec;
	std::filesystem::rename(temp_path, output, ec);
	if (ec)
	{
		throw std::runtime_error("failed to write output file");
	}
}

template <typename Options, typename Operation>
void run_streaming_file_command(const Options &options, Operation operation)
{
	if (options.input_is_stdin)
	{
		set_stdin_binary();
	}
	if (options.output_is_stdout)
	{
		set_stdout_binary();
	}

	std::ifstream input_file;
	std::ofstream output_file;
	std::filesystem::path temp_path;
	bool uses_temp_output = false;

	try
	{
		std::istream *input = &std::cin;
		if (!options.input_is_stdin)
		{
			input_file = open_cli_input_file(options.input);
			input = &input_file;
		}

		std::ostream *output = &std::cout;
		if (!options.output_is_stdout)
		{
			ensure_output_file_does_not_exist(options.output);
			temp_path = temporary_output_path_for(options.output);
			output_file = open_cli_output_file(temp_path);
			uses_temp_output = true;
			output = &output_file;
		}

		operation(*input, *output);

		if (options.output_is_stdout)
		{
			std::cout.flush();
			if (!std::cout)
			{
				throw std::runtime_error("failed to write output file");
			}
			return;
		}

		output_file.close();
		if (!output_file)
		{
			throw std::runtime_error("failed to write output file");
		}
		rename_output_file(temp_path, options.output);
		uses_temp_output = false;
	}
	catch (...)
	{
		if (output_file.is_open())
		{
			output_file.close();
		}
		if (uses_temp_output)
		{
			remove_file_quietly(temp_path);
		}
		throw;
	}
}

wrap_key_options parse_wrap_key_options(int argc, char **argv)
{
	if (argc < 6 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(wrap_key_usage()));
	}

	wrap_key_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--key-hex" || option == "--key-file")
		{
			if (options.has_key)
			{
				throw std::runtime_error("conflicting key options");
			}
			options.key = key_from_option(option, value);
			options.has_key = true;
		}
		else if (option == "--wrapping-key-hex" || option == "--wrapping-key-file")
		{
			if (options.has_wrapping_key)
			{
				throw std::runtime_error("conflicting wrapping key options");
			}
			options.wrapping_key = wrapping_key_from_option(option, value);
			options.has_wrapping_key = true;
		}
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.has_output = true;
		}
		else
		{
			throw std::runtime_error(std::string("unsupported key wrapping option: ") + std::string(option));
		}
	}

	if (!options.has_key || !options.has_wrapping_key)
	{
		throw std::runtime_error(std::string(wrap_key_usage()));
	}

	return options;
}

unwrap_key_options parse_unwrap_key_options(int argc, char **argv)
{
	if (argc < 6 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(unwrap_key_usage()));
	}

	unwrap_key_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--packet-hex" || option == "--packet-file")
		{
			if (options.has_packet)
			{
				throw std::runtime_error("conflicting packet options");
			}
			options.packet = option == "--packet-hex" ? securekit::hex_decode(value) : read_file(std::filesystem::path(value));
			options.has_packet = true;
		}
		else if (option == "--wrapping-key-hex" || option == "--wrapping-key-file")
		{
			if (options.has_wrapping_key)
			{
				throw std::runtime_error("conflicting wrapping key options");
			}
			options.wrapping_key = wrapping_key_from_option(option, value);
			options.has_wrapping_key = true;
		}
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.has_output = true;
		}
		else
		{
			throw std::runtime_error(std::string("unsupported key wrapping option: ") + std::string(option));
		}
	}

	if (!options.has_packet || !options.has_wrapping_key)
	{
		throw std::runtime_error(std::string(unwrap_key_usage()));
	}

	return options;
}

encrypt_options parse_encrypt_options(int argc, char **argv)
{
	if (argc < 6 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(packet_command_usage(argv[1])));
	}

	encrypt_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--text" || option == "--in")
		{
			if (options.has_plaintext)
			{
				throw std::runtime_error("conflicting input options");
			}
			options.plaintext = option == "--text" ? bytes_from_text(value) : read_file(std::filesystem::path(value));
			options.has_plaintext = true;
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
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.has_output = true;
		}
		else
		{
			throw std::runtime_error(std::string("unsupported packet option: ") + std::string(option));
		}
	}

	if (!options.has_plaintext || !options.has_key)
	{
		throw std::runtime_error(std::string(packet_command_usage(argv[1])));
	}

	return options;
}

decrypt_options parse_decrypt_options(int argc, char **argv)
{
	if (argc < 6 || ((argc - 2) % 2) != 0)
	{
		throw std::runtime_error(std::string(packet_command_usage(argv[1])));
	}

	decrypt_options options;
	for (int index = 2; index < argc; index += 2)
	{
		const std::string_view option = argv[index];
		const std::string_view value = argv[index + 1];

		if (option == "--packet-hex" || option == "--packet-file")
		{
			if (options.has_packet)
			{
				throw std::runtime_error("conflicting packet options");
			}
			options.packet = option == "--packet-hex" ? securekit::hex_decode(value) : read_file(std::filesystem::path(value));
			options.has_packet = true;
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
		else if (option == "--out")
		{
			reject_duplicate(options.has_output, option);
			options.output = std::filesystem::path(value);
			options.has_output = true;
		}
		else
		{
			throw std::runtime_error(std::string("unsupported packet option: ") + std::string(option));
		}
	}

	if (!options.has_packet || !options.has_key)
	{
		throw std::runtime_error(std::string(packet_command_usage(argv[1])));
	}

	return options;
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

		if (argc == 6 && is_arg(argv[1], "hmac-sha256") && is_arg(argv[2], "--key-hex") &&
		    is_arg(argv[4], "--text"))
		{
			std::cout << securekit::hex_encode(
			                 securekit::hmac_sha256(securekit::hex_decode(argv[3]), bytes_from_text(argv[5])))
			          << '\n';
			return 0;
		}

		if (argc == 6 && is_arg(argv[1], "hmac-sha256") && is_arg(argv[2], "--key-hex") &&
		    is_arg(argv[4], "--file"))
		{
			std::cout
			    << securekit::hex_encode(securekit::hmac_sha256(securekit::hex_decode(argv[3]), read_file(argv[5])))
			    << '\n';
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "hmac-sha256"))
		{
			return fail(hmac_sha256_usage());
		}

		if (argc == 10 && is_arg(argv[1], "hkdf-sha256") && is_arg(argv[2], "--key-hex") &&
		    is_arg(argv[4], "--salt-hex") && is_arg(argv[8], "--out-size") &&
		    (is_arg(argv[6], "--info-hex") || is_arg(argv[6], "--info-text")))
		{
			const securekit::bytes info = is_arg(argv[6], "--info-hex") ? securekit::hex_decode(argv[7])
			                                                            : bytes_from_text(argv[7]);
			std::cout << securekit::hex_encode(securekit::hkdf_sha256(
			                 securekit::hex_decode(argv[3]), securekit::hex_decode(argv[5]), info, parse_size(argv[9])))
			          << '\n';
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "hkdf-sha256"))
		{
			return fail(hkdf_sha256_usage());
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

		if (argc >= 2 && is_arg(argv[1], "wrap-key"))
		{
			const wrap_key_options options = parse_wrap_key_options(argc, argv);
			const securekit::bytes packet = securekit::wrap_key(options.key, options.wrapping_key);
			if (options.has_output)
			{
				write_binary_file(options.output, packet);
			}
			else
			{
				std::cout << securekit::hex_encode(packet) << '\n';
			}
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "unwrap-key"))
		{
			const unwrap_key_options options = parse_unwrap_key_options(argc, argv);
			const securekit::key256 key = securekit::unwrap_key(options.packet, options.wrapping_key);
			if (options.has_output)
			{
				write_key_file(options.output, key);
			}
			else
			{
				std::cout << securekit::hex_encode(key) << '\n';
			}
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "encrypt"))
		{
			const encrypt_options options = parse_encrypt_options(argc, argv);
			const securekit::bytes packet = securekit::encrypt(options.plaintext, options.key, options.aad);
			if (options.has_output)
			{
				write_binary_file(options.output, packet);
			}
			else
			{
				std::cout << securekit::hex_encode(packet) << '\n';
			}
			return 0;
		}

		if (argc >= 2 && is_arg(argv[1], "decrypt"))
		{
			const decrypt_options options = parse_decrypt_options(argc, argv);
			const securekit::bytes plaintext = securekit::decrypt(options.packet, options.key, options.aad);
			if (options.has_output)
			{
				write_binary_file(options.output, plaintext);
			}
			else
			{
				write_bytes(plaintext);
			}
			return 0;
		}

		if (argc >= 2 && (is_arg(argv[1], "seal-file") || is_arg(argv[1], "open-file")))
		{
			const file_command_options options = parse_file_command_options(argc, argv);
			const bool uses_standard_stream = options.input_is_stdin || options.output_is_stdout;

			if (is_arg(argv[1], "seal-file"))
			{
				if (uses_standard_stream)
				{
					run_streaming_file_command(options, [&](std::istream &input, std::ostream &output) {
						securekit::seal_file(input, output, options.key, options.aad);
					});
				}
				else
				{
					securekit::seal_file(options.input, options.output, options.key, options.aad);
				}
			}
			else
			{
				if (uses_standard_stream)
				{
					run_streaming_file_command(options, [&](std::istream &input, std::ostream &output) {
						securekit::open_file(input, output, options.key, options.aad);
					});
				}
				else
				{
					securekit::open_file(options.input, options.output, options.key, options.aad);
				}
			}
			return 0;
		}

		if (argc >= 2 && (is_arg(argv[1], "seal-file-password") || is_arg(argv[1], "open-file-password")))
		{
			const password_file_command_options options = parse_password_file_command_options(argc, argv);
			const bool uses_standard_stream = options.input_is_stdin || options.output_is_stdout;

			if (is_arg(argv[1], "seal-file-password"))
			{
				if (uses_standard_stream)
				{
					run_streaming_file_command(options, [&](std::istream &input, std::ostream &output) {
						securekit::seal_file_with_password(input, output, options.password, options.aad);
					});
				}
				else
				{
					securekit::seal_file_with_password(options.input, options.output, options.password, options.aad);
				}
			}
			else
			{
				if (uses_standard_stream)
				{
					run_streaming_file_command(options, [&](std::istream &input, std::ostream &output) {
						securekit::open_file_with_password(input, output, options.password, options.aad);
					});
				}
				else
				{
					securekit::open_file_with_password(options.input, options.output, options.password, options.aad);
				}
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
