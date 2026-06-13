#include "securekit/hash.hpp"

#include <array>
#include <memory>

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>

#include "securekit/error.hpp"

namespace
{

using DigestContext = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using Kdf = std::unique_ptr<EVP_KDF, decltype(&EVP_KDF_free)>;
using KdfContext = std::unique_ptr<EVP_KDF_CTX, decltype(&EVP_KDF_CTX_free)>;

[[noreturn]] void throw_backend_failure()
{
	throw securekit::error(securekit::error_code::backend_failure, "OpenSSL SHA-256 operation failed");
}

[[noreturn]] void throw_hmac_backend_failure()
{
	throw securekit::error(securekit::error_code::backend_failure, "OpenSSL HMAC-SHA-256 operation failed");
}

[[noreturn]] void throw_hkdf_backend_failure()
{
	throw securekit::error(securekit::error_code::backend_failure, "OpenSSL HKDF-SHA-256 operation failed");
}

} // namespace

namespace securekit
{

digest256 sha256(std::span<const std::byte> input)
{
	DigestContext context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
	const EVP_MD *digest = EVP_sha256();
	if (context == nullptr || digest == nullptr)
	{
		throw_backend_failure();
	}

	if (EVP_DigestInit_ex(context.get(), digest, nullptr) != 1)
	{
		throw_backend_failure();
	}

	const bool digest_updated = input.empty() || EVP_DigestUpdate(context.get(), input.data(), input.size()) == 1;
	if (!digest_updated)
	{
		throw_backend_failure();
	}

	digest256 output{};
	unsigned int output_size = 0;
	auto *output_data = reinterpret_cast<unsigned char *>(output.data());
	if (EVP_DigestFinal_ex(context.get(), output_data, &output_size) != 1)
	{
		throw_backend_failure();
	}

	if (output_size != static_cast<unsigned int>(output.size()))
	{
		throw_backend_failure();
	}

	return output;
}

bytes hkdf_sha256(std::span<const std::byte> key_material, std::span<const std::byte> salt, std::span<const std::byte> info, std::size_t output_size)
{
	bytes output(output_size);
	if (output.empty())
	{
		return output;
	}

	Kdf kdf(EVP_KDF_fetch(nullptr, "HKDF", nullptr), EVP_KDF_free);
	if (kdf == nullptr)
	{
		throw_hkdf_backend_failure();
	}

	KdfContext context(EVP_KDF_CTX_new(kdf.get()), EVP_KDF_CTX_free);
	if (context == nullptr)
	{
		throw_hkdf_backend_failure();
	}

	char digest_name[] = "SHA256";
	int mode = EVP_KDF_HKDF_MODE_EXTRACT_AND_EXPAND;
	std::byte empty_octet{};
	auto *key_data = const_cast<std::byte *>(key_material.empty() ? &empty_octet : key_material.data());

	std::array<OSSL_PARAM, 6> params{};
	std::size_t param_count = 0;
	params[param_count++] = OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, digest_name, 0);
	params[param_count++] = OSSL_PARAM_construct_int(OSSL_KDF_PARAM_MODE, &mode);
	params[param_count++] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, key_data, key_material.size());
	if (!salt.empty())
	{
		params[param_count++] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, const_cast<std::byte *>(salt.data()), salt.size());
	}
	if (!info.empty())
	{
		params[param_count++] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_INFO, const_cast<std::byte *>(info.data()), info.size());
	}
	params[param_count] = OSSL_PARAM_construct_end();

	auto *output_data = reinterpret_cast<unsigned char *>(output.data());
	if (EVP_KDF_derive(context.get(), output_data, output.size(), params.data()) != 1)
	{
		throw_hkdf_backend_failure();
	}

	return output;
}

digest256 hmac_sha256(std::span<const std::byte> key, std::span<const std::byte> input)
{
	digest256 output{};
	std::size_t output_size = 0;

	auto *output_data = reinterpret_cast<unsigned char *>(output.data());
	const auto *input_data = reinterpret_cast<const unsigned char *>(input.data());

	if (EVP_Q_mac(
	        nullptr,
	        "HMAC",
	        nullptr,
	        "SHA256",
	        nullptr,
	        key.data(),
	        key.size(),
	        input_data,
	        input.size(),
	        output_data,
	        output.size(),
	        &output_size) == nullptr)
	{
		throw_hmac_backend_failure();
	}

	if (output_size != output.size())
	{
		throw_hmac_backend_failure();
	}

	return output;
}

} // namespace securekit
