#include "securekit/aead.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <span>
#include <string>

#include "securekit/error.hpp"

namespace
{

constexpr std::array<std::byte, 4> kMagic{std::byte{'S'}, std::byte{'K'}, std::byte{'T'}, std::byte{'1'}};
constexpr std::byte kVersion{0x01};
constexpr std::size_t kMagicSize = kMagic.size();
constexpr std::size_t kVersionSize = 1;
constexpr std::size_t kHeaderSize = kMagicSize + kVersionSize;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kOverhead = kHeaderSize + kNonceSize + kTagSize;

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

[[noreturn]] void throw_backend_failure()
{
	throw securekit::error(securekit::error_code::backend_failure, "OpenSSL AES-256-GCM operation failed");
}

[[noreturn]] void throw_invalid_packet()
{
	throw securekit::error(securekit::error_code::invalid_packet, "Invalid AEAD packet");
}

[[noreturn]] void throw_authentication_failed()
{
	throw securekit::error(securekit::error_code::authentication_failed, "AEAD authentication failed");
}

void check_update_size(std::size_t size, const char *name)
{
	if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		throw securekit::error(securekit::error_code::invalid_input, std::string(name) + " exceeds OpenSSL update limit");
	}
}

int update_size(std::span<const std::byte> input, const char *name)
{
	check_update_size(input.size(), name);
	return static_cast<int>(input.size());
}

const unsigned char *openssl_data(std::span<const std::byte> input)
{
	return reinterpret_cast<const unsigned char *>(input.data());
}

unsigned char *openssl_data(std::byte *input)
{
	return reinterpret_cast<unsigned char *>(input);
}

std::array<std::byte, kHeaderSize> make_header()
{
	std::array<std::byte, kHeaderSize> header{};
	std::copy(kMagic.begin(), kMagic.end(), header.begin());
	header[kMagicSize] = kVersion;
	return header;
}

void require_valid_header(std::span<const std::byte> packet)
{
	if (packet.size() < kOverhead)
	{
		throw_invalid_packet();
	}

	if (!std::equal(kMagic.begin(), kMagic.end(), packet.begin()))
	{
		throw_invalid_packet();
	}

	if (packet[kMagicSize] != kVersion)
	{
		throw_invalid_packet();
	}
}

CipherContext make_context()
{
	CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
	if (context == nullptr || EVP_aes_256_gcm() == nullptr)
	{
		throw_backend_failure();
	}
	return context;
}

bool initialize_encrypt_context(EVP_CIPHER_CTX *context, const securekit::key256 &key, std::span<const std::byte> nonce)
{
	const auto key_bytes = std::span<const std::byte>(key);
	const bool cipher_initialized = EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
	const bool nonce_size_set = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(kNonceSize), nullptr) == 1;
	const bool key_and_nonce_initialized = EVP_EncryptInit_ex(context, nullptr, nullptr, openssl_data(key_bytes), openssl_data(nonce)) == 1;

	return cipher_initialized && nonce_size_set && key_and_nonce_initialized;
}

bool initialize_decrypt_context(EVP_CIPHER_CTX *context, const securekit::key256 &key, std::span<const std::byte> nonce)
{
	const auto key_bytes = std::span<const std::byte>(key);
	const bool cipher_initialized = EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
	const bool nonce_size_set = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(kNonceSize), nullptr) == 1;
	const bool key_and_nonce_initialized = EVP_DecryptInit_ex(context, nullptr, nullptr, openssl_data(key_bytes), openssl_data(nonce)) == 1;

	return cipher_initialized && nonce_size_set && key_and_nonce_initialized;
}

void update_aad(EVP_CIPHER_CTX *context, std::span<const std::byte> aad, const char *name)
{
	const int aad_size = update_size(aad, name);
	if (aad_size == 0)
	{
		return;
	}

	int unused = 0;
	if (EVP_CipherUpdate(context, nullptr, &unused, openssl_data(aad), aad_size) != 1)
	{
		throw_backend_failure();
	}
}

bool encrypt_plaintext(EVP_CIPHER_CTX *context, std::span<const std::byte> plaintext, std::byte *ciphertext, int &ciphertext_size)
{
	if (plaintext.empty())
	{
		return true;
	}

	const int plaintext_size = static_cast<int>(plaintext.size());
	return EVP_EncryptUpdate(context, openssl_data(ciphertext), &ciphertext_size, openssl_data(plaintext), plaintext_size) == 1;
}

bool decrypt_ciphertext(EVP_CIPHER_CTX *context, std::span<const std::byte> ciphertext, std::byte *plaintext, int &plaintext_size)
{
	if (ciphertext.empty())
	{
		return true;
	}

	const int ciphertext_size = static_cast<int>(ciphertext.size());
	return EVP_DecryptUpdate(context, openssl_data(plaintext), &plaintext_size, openssl_data(ciphertext), ciphertext_size) == 1;
}

} // namespace

namespace securekit
{

bytes encrypt(std::span<const std::byte> plaintext, const key256 &key, std::span<const std::byte> aad)
{
	const int plaintext_size = update_size(plaintext, "plaintext");
	check_update_size(aad.size(), "AAD");

	bytes packet(kOverhead + plaintext.size());
	const auto header = make_header();
	std::copy(header.begin(), header.end(), packet.begin());

	std::byte *nonce = packet.data() + kHeaderSize;
	if (RAND_bytes(openssl_data(nonce), static_cast<int>(kNonceSize)) != 1)
	{
		throw_backend_failure();
	}

	CipherContext context = make_context();
	if (!initialize_encrypt_context(context.get(), key, std::span<const std::byte>(nonce, kNonceSize)))
	{
		throw_backend_failure();
	}

	update_aad(context.get(), header, "header");
	update_aad(context.get(), aad, "AAD");

	int ciphertext_size = 0;
	std::byte *ciphertext = packet.data() + kHeaderSize + kNonceSize;
	if (!encrypt_plaintext(context.get(), plaintext, ciphertext, ciphertext_size))
	{
		throw_backend_failure();
	}

	int final_size = 0;
	if (EVP_EncryptFinal_ex(context.get(), openssl_data(ciphertext + ciphertext_size), &final_size) != 1)
	{
		throw_backend_failure();
	}

	if (ciphertext_size + final_size != plaintext_size)
	{
		throw_backend_failure();
	}

	std::byte *tag = packet.data() + kHeaderSize + kNonceSize + plaintext.size();
	if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(kTagSize), openssl_data(tag)) != 1)
	{
		throw_backend_failure();
	}

	return packet;
}

bytes decrypt(std::span<const std::byte> packet, const key256 &key, std::span<const std::byte> aad)
{
	require_valid_header(packet);

	const std::size_t ciphertext_byte_size = packet.size() - kOverhead;
	check_update_size(ciphertext_byte_size, "ciphertext");
	check_update_size(aad.size(), "AAD");

	const std::span<const std::byte> header = packet.subspan(0, kHeaderSize);
	const std::span<const std::byte> nonce = packet.subspan(kHeaderSize, kNonceSize);
	const std::span<const std::byte> ciphertext = packet.subspan(kHeaderSize + kNonceSize, ciphertext_byte_size);
	const std::span<const std::byte> packet_tag = packet.subspan(packet.size() - kTagSize, kTagSize);

	bytes plaintext(ciphertext_byte_size);

	CipherContext context = make_context();
	if (!initialize_decrypt_context(context.get(), key, nonce))
	{
		throw_backend_failure();
	}

	update_aad(context.get(), header, "header");
	update_aad(context.get(), aad, "AAD");

	int plaintext_size = 0;
	const int ciphertext_size = static_cast<int>(ciphertext.size());
	if (!decrypt_ciphertext(context.get(), ciphertext, plaintext.data(), plaintext_size))
	{
		throw_backend_failure();
	}

	std::array<std::byte, kTagSize> tag{};
	std::memcpy(tag.data(), packet_tag.data(), tag.size());
	const int tag_size = static_cast<int>(tag.size());
	if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_TAG, tag_size, openssl_data(tag.data())) != 1)
	{
		throw_backend_failure();
	}

	std::array<std::byte, 1> final_buffer{};
	std::byte *final_output = plaintext.empty() ? final_buffer.data() : plaintext.data() + plaintext_size;
	int final_size = 0;
	if (EVP_DecryptFinal_ex(context.get(), openssl_data(final_output), &final_size) != 1)
	{
		std::fill(plaintext.begin(), plaintext.end(), std::byte{0});
		throw_authentication_failed();
	}

	if (plaintext_size + final_size != ciphertext_size)
	{
		throw_backend_failure();
	}

	return plaintext;
}

} // namespace securekit
