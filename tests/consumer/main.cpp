#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

#include "securekit/securekit.hpp"

namespace
{

securekit::bytes ascii_bytes(std::string_view text)
{
	securekit::bytes result;
	result.reserve(text.size());
	for (const char ch : text)
	{
		result.push_back(static_cast<std::byte>(ch));
	}
	return result;
}

void write_file(const std::filesystem::path &path, const securekit::bytes &bytes)
{
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

securekit::bytes read_file(const std::filesystem::path &path)
{
	std::ifstream in(path, std::ios::binary);
	const std::string data{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
	securekit::bytes result;
	result.reserve(data.size());
	for (const unsigned char ch : data)
	{
		result.push_back(static_cast<std::byte>(ch));
	}
	return result;
}

} // namespace

int main()
{
	const auto plaintext = ascii_bytes("SecureKit consumer sample plaintext");
	const auto aad = ascii_bytes("SecureKit consumer sample aad");
	const auto key = securekit::generate_key();

	const auto packet = securekit::encrypt(plaintext, key, aad);
	const auto roundtrip = securekit::decrypt(packet, key, aad);
	const std::string token = securekit::random_token(32);

	const auto plain_path = std::filesystem::temp_directory_path() / "securekit-consumer-plain.bin";
	const auto sealed_path = std::filesystem::temp_directory_path() / "securekit-consumer-sealed.skf";
	const auto opened_path = std::filesystem::temp_directory_path() / "securekit-consumer-opened.bin";
	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	write_file(plain_path, plaintext);
	securekit::seal_file(plain_path, sealed_path, key, aad);
	securekit::open_file(sealed_path, opened_path, key, aad);
	const auto opened = read_file(opened_path);

	std::filesystem::remove(plain_path);
	std::filesystem::remove(sealed_path);
	std::filesystem::remove(opened_path);

	return roundtrip == plaintext && opened == plaintext && !token.empty() ? 0 : 1;
}
