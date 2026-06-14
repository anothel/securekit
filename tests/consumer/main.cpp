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

	const auto from_hex = securekit::hex_decode(securekit::hex_encode(plaintext));
	const auto from_base64 = securekit::base64_decode(securekit::base64_encode(from_hex));
	const auto from_base64url = securekit::base64url_decode(securekit::base64url_encode(from_base64));
	const auto digest = securekit::sha256(ascii_bytes("abc"));
	const auto hmac = securekit::hmac_sha256(ascii_bytes("key"), ascii_bytes("The quick brown fox jumps over the lazy dog"));
	const auto hkdf = securekit::hkdf_sha256(ascii_bytes("ikm"), ascii_bytes("salt"), ascii_bytes("info"), 16);
	const auto random = securekit::random_bytes(8);

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

	const bool utility_api_ok = from_base64url == plaintext &&
	                            securekit::hex_encode(digest) ==
	                                "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" &&
	                            securekit::hex_encode(hmac) ==
	                                "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8" &&
	                            hkdf.size() == 16 && random.size() == 8 && securekit::constant_time_equal(digest, digest);

	return utility_api_ok && roundtrip == plaintext && opened == plaintext && !token.empty() ? 0 : 1;
}
