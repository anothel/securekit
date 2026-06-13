#include "securekit/hash.hpp"

#include <memory>

#include <openssl/evp.h>

#include "securekit/error.hpp"

namespace
{

using DigestContext = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

[[noreturn]] void throw_backend_failure()
{
	throw securekit::error(securekit::error_code::backend_failure, "OpenSSL SHA-256 operation failed");
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

} // namespace securekit
