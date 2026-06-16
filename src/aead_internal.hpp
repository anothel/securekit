#ifndef SECUREKIT_SRC_AEAD_INTERNAL_HPP_
#define SECUREKIT_SRC_AEAD_INTERNAL_HPP_

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <string>

#include "securekit/error.hpp"
#include "securekit/types.hpp"

namespace securekit::internal_aead
{

inline constexpr std::array<std::byte, 4> kMagic{std::byte{'S'}, std::byte{'K'}, std::byte{'T'}, std::byte{'1'}};
inline constexpr std::byte kVersion{0x01};
inline constexpr std::size_t kMagicSize = kMagic.size();
inline constexpr std::size_t kVersionSize = 1;
inline constexpr std::size_t kHeaderSize = kMagicSize + kVersionSize;
inline constexpr std::size_t kNonceSize = 12;
inline constexpr std::size_t kPrefixSize = kHeaderSize + kNonceSize;
inline constexpr std::size_t kTagSize = 16;
inline constexpr std::size_t kOverhead = kPrefixSize + kTagSize;

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

[[noreturn]] inline void throw_backend_failure()
{
	throw error(error_code::backend_failure, "OpenSSL AES-256-GCM operation failed");
}

[[noreturn]] inline void throw_invalid_packet()
{
	throw error(error_code::invalid_packet, "Invalid AEAD packet");
}

[[noreturn]] inline void throw_authentication_failed()
{
	throw error(error_code::authentication_failed, "AEAD authentication failed");
}

[[noreturn]] inline void throw_invalid_input(const char *message)
{
	throw error(error_code::invalid_input, message);
}

inline void check_update_size(std::size_t size, const char *name)
{
	if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		throw error(error_code::invalid_input, std::string(name) + " exceeds OpenSSL update limit");
	}
}

inline int update_size(std::span<const std::byte> input, const char *name)
{
	check_update_size(input.size(), name);
	return static_cast<int>(input.size());
}

inline const unsigned char *openssl_data(std::span<const std::byte> input)
{
	return reinterpret_cast<const unsigned char *>(input.data());
}

inline unsigned char *openssl_data(std::byte *input)
{
	return reinterpret_cast<unsigned char *>(input);
}

inline std::array<std::byte, kHeaderSize> make_header()
{
	std::array<std::byte, kHeaderSize> header{};
	std::copy(kMagic.begin(), kMagic.end(), header.begin());
	header[kMagicSize] = kVersion;
	return header;
}

inline bytes make_packet_prefix(std::span<const std::byte> nonce)
{
	if (nonce.size() != kNonceSize)
	{
		throw_backend_failure();
	}

	bytes prefix(kPrefixSize);
	const auto header = make_header();
	std::copy(header.begin(), header.end(), prefix.begin());
	std::copy(nonce.begin(), nonce.end(), prefix.begin() + kHeaderSize);
	return prefix;
}

inline void require_valid_prefix(std::span<const std::byte> packet_prefix)
{
	if (packet_prefix.size() != kPrefixSize)
	{
		throw_invalid_packet();
	}

	if (!std::equal(kMagic.begin(), kMagic.end(), packet_prefix.begin()))
	{
		throw_invalid_packet();
	}

	if (packet_prefix[kMagicSize] != kVersion)
	{
		throw_invalid_packet();
	}
}

inline void require_valid_packet(std::span<const std::byte> packet)
{
	if (packet.size() < kOverhead)
	{
		throw_invalid_packet();
	}

	require_valid_prefix(packet.first(kPrefixSize));
}

inline CipherContext make_context()
{
	CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
	if (context == nullptr || EVP_aes_256_gcm() == nullptr)
	{
		throw_backend_failure();
	}
	return context;
}

inline bool initialize_encrypt_context(EVP_CIPHER_CTX *context, const key256 &key, std::span<const std::byte> nonce)
{
	const auto key_bytes = std::span<const std::byte>(key);
	const bool cipher_initialized = EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
	const bool nonce_size_set = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(kNonceSize), nullptr) == 1;
	const bool key_and_nonce_initialized = EVP_EncryptInit_ex(context, nullptr, nullptr, openssl_data(key_bytes), openssl_data(nonce)) == 1;

	return cipher_initialized && nonce_size_set && key_and_nonce_initialized;
}

inline bool initialize_decrypt_context(EVP_CIPHER_CTX *context, const key256 &key, std::span<const std::byte> nonce)
{
	const auto key_bytes = std::span<const std::byte>(key);
	const bool cipher_initialized = EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
	const bool nonce_size_set = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(kNonceSize), nullptr) == 1;
	const bool key_and_nonce_initialized = EVP_DecryptInit_ex(context, nullptr, nullptr, openssl_data(key_bytes), openssl_data(nonce)) == 1;

	return cipher_initialized && nonce_size_set && key_and_nonce_initialized;
}

inline void update_aad(EVP_CIPHER_CTX *context, std::span<const std::byte> aad, const char *name)
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

} // namespace securekit::internal_aead

#endif // SECUREKIT_SRC_AEAD_INTERNAL_HPP_
