#include "securekit/packet_stream.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>

#include "aead_internal.hpp"
#include "wipe.hpp"

namespace
{

enum class packet_stream_phase
{
	ready,
	active,
	finished,
};

securekit::bytes copy_bytes(std::span<const std::byte> input)
{
	return securekit::bytes(input.begin(), input.end());
}

securekit::bytes encrypt_chunk(EVP_CIPHER_CTX *context, std::span<const std::byte> plaintext)
{
	const int plaintext_size = securekit::internal_aead::update_size(plaintext, "plaintext");
	securekit::bytes ciphertext(plaintext.size());

	if (plaintext_size == 0)
	{
		return ciphertext;
	}

	int ciphertext_size = 0;
	if (EVP_EncryptUpdate(context,
	        securekit::internal_aead::openssl_data(ciphertext.data()),
	        &ciphertext_size,
	        securekit::internal_aead::openssl_data(plaintext),
	        plaintext_size) != 1)
	{
		securekit::internal_aead::throw_backend_failure();
	}

	if (ciphertext_size != plaintext_size)
	{
		securekit::internal_aead::throw_backend_failure();
	}

	return ciphertext;
}

securekit::bytes decrypt_chunk(EVP_CIPHER_CTX *context, std::span<const std::byte> ciphertext)
{
	const int ciphertext_size = securekit::internal_aead::update_size(ciphertext, "ciphertext");
	securekit::bytes plaintext(ciphertext.size());

	if (ciphertext_size == 0)
	{
		return plaintext;
	}

	int plaintext_size = 0;
	if (EVP_DecryptUpdate(context,
	        securekit::internal_aead::openssl_data(plaintext.data()),
	        &plaintext_size,
	        securekit::internal_aead::openssl_data(ciphertext),
	        ciphertext_size) != 1)
	{
		securekit::internal_aead::throw_backend_failure();
	}

	if (plaintext_size != ciphertext_size)
	{
		securekit::internal_aead::throw_backend_failure();
	}

	return plaintext;
}

} // namespace

namespace securekit
{

struct packet_encryptor::impl
{
	impl(const key256 &input_key, std::span<const std::byte> input_aad)
	    : key(input_key),
	      aad(copy_bytes(input_aad)),
	      context(nullptr, EVP_CIPHER_CTX_free)
	{
		internal_aead::check_update_size(aad.size(), "AAD");
	}

	~impl()
	{
		context.reset();
		internal::secure_wipe(key);
		internal::secure_wipe(aad);
	}

	key256 key{};
	bytes aad;
	internal_aead::CipherContext context;
	packet_stream_phase phase = packet_stream_phase::ready;
};

struct packet_decryptor::impl
{
	impl(const key256 &input_key, std::span<const std::byte> input_aad)
	    : key(input_key),
	      aad(copy_bytes(input_aad)),
	      context(nullptr, EVP_CIPHER_CTX_free)
	{
		internal_aead::check_update_size(aad.size(), "AAD");
	}

	~impl()
	{
		context.reset();
		internal::secure_wipe(key);
		internal::secure_wipe(aad);
	}

	key256 key{};
	bytes aad;
	internal_aead::CipherContext context;
	packet_stream_phase phase = packet_stream_phase::ready;
};

packet_encryptor::packet_encryptor(const key256 &key, std::span<const std::byte> aad) : impl_(std::make_unique<impl>(key, aad))
{
}

packet_encryptor::~packet_encryptor() = default;

packet_encryptor::packet_encryptor(packet_encryptor &&other) noexcept = default;

packet_encryptor &packet_encryptor::operator=(packet_encryptor &&other) noexcept = default;

bytes packet_encryptor::begin()
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet encryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::ready)
	{
		internal_aead::throw_invalid_input("packet encryptor begin() may only be called once");
	}

	bytes nonce(internal_aead::kNonceSize);
	if (RAND_bytes(internal_aead::openssl_data(nonce.data()), static_cast<int>(nonce.size())) != 1)
	{
		internal_aead::throw_backend_failure();
	}

	impl_->context = internal_aead::make_context();
	if (!internal_aead::initialize_encrypt_context(impl_->context.get(), impl_->key, nonce))
	{
		internal_aead::throw_backend_failure();
	}

	const auto header = internal_aead::make_header();
	internal_aead::update_aad(impl_->context.get(), header, "header");
	internal_aead::update_aad(impl_->context.get(), impl_->aad, "AAD");

	impl_->phase = packet_stream_phase::active;
	return internal_aead::make_packet_prefix(nonce);
}

bytes packet_encryptor::update(std::span<const std::byte> plaintext)
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet encryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::active)
	{
		internal_aead::throw_invalid_input("packet encryptor update() requires begin() before finalize()");
	}

	return encrypt_chunk(impl_->context.get(), plaintext);
}

bytes packet_encryptor::finalize()
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet encryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::active)
	{
		internal_aead::throw_invalid_input("packet encryptor finalize() requires begin() exactly once");
	}

	std::array<std::byte, 1> final_buffer{};
	int final_size = 0;
	if (EVP_EncryptFinal_ex(impl_->context.get(), internal_aead::openssl_data(final_buffer.data()), &final_size) != 1)
	{
		internal_aead::throw_backend_failure();
	}

	if (final_size != 0)
	{
		internal_aead::throw_backend_failure();
	}

	bytes tag(internal_aead::kTagSize);
	if (EVP_CIPHER_CTX_ctrl(impl_->context.get(),
	        EVP_CTRL_GCM_GET_TAG,
	        static_cast<int>(tag.size()),
	        internal_aead::openssl_data(tag.data())) != 1)
	{
		internal_aead::throw_backend_failure();
	}

	impl_->context.reset();
	impl_->phase = packet_stream_phase::finished;
	return tag;
}

packet_decryptor::packet_decryptor(const key256 &key, std::span<const std::byte> aad) : impl_(std::make_unique<impl>(key, aad))
{
}

packet_decryptor::~packet_decryptor() = default;

packet_decryptor::packet_decryptor(packet_decryptor &&other) noexcept = default;

packet_decryptor &packet_decryptor::operator=(packet_decryptor &&other) noexcept = default;

void packet_decryptor::begin(std::span<const std::byte> packet_prefix)
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet decryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::ready)
	{
		internal_aead::throw_invalid_input("packet decryptor begin() may only be called once");
	}

	internal_aead::require_valid_prefix(packet_prefix);

	const std::span<const std::byte> header = packet_prefix.first(internal_aead::kHeaderSize);
	const std::span<const std::byte> nonce = packet_prefix.last(internal_aead::kNonceSize);

	impl_->context = internal_aead::make_context();
	if (!internal_aead::initialize_decrypt_context(impl_->context.get(), impl_->key, nonce))
	{
		internal_aead::throw_backend_failure();
	}

	internal_aead::update_aad(impl_->context.get(), header, "header");
	internal_aead::update_aad(impl_->context.get(), impl_->aad, "AAD");

	impl_->phase = packet_stream_phase::active;
}

bytes packet_decryptor::update(std::span<const std::byte> ciphertext)
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet decryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::active)
	{
		internal_aead::throw_invalid_input("packet decryptor update() requires begin() before finalize()");
	}

	return decrypt_chunk(impl_->context.get(), ciphertext);
}

bytes packet_decryptor::finalize(std::span<const std::byte> tag)
{
	if (impl_ == nullptr)
	{
		internal_aead::throw_invalid_input("packet decryptor has no state");
	}

	if (impl_->phase != packet_stream_phase::active)
	{
		internal_aead::throw_invalid_input("packet decryptor finalize() requires begin() exactly once");
	}

	if (tag.size() != internal_aead::kTagSize)
	{
		impl_->context.reset();
		impl_->phase = packet_stream_phase::finished;
		internal_aead::throw_invalid_packet();
	}

	std::array<std::byte, internal_aead::kTagSize> tag_copy{};
	std::copy(tag.begin(), tag.end(), tag_copy.begin());
	if (EVP_CIPHER_CTX_ctrl(impl_->context.get(),
	        EVP_CTRL_GCM_SET_TAG,
	        static_cast<int>(tag_copy.size()),
	        internal_aead::openssl_data(tag_copy.data())) != 1)
	{
		internal_aead::throw_backend_failure();
	}

	std::array<std::byte, 1> final_buffer{};
	int final_size = 0;
	if (EVP_DecryptFinal_ex(impl_->context.get(), internal_aead::openssl_data(final_buffer.data()), &final_size) != 1)
	{
		impl_->context.reset();
		impl_->phase = packet_stream_phase::finished;
		internal_aead::throw_authentication_failed();
	}

	if (final_size < 0 || static_cast<std::size_t>(final_size) > final_buffer.size())
	{
		internal_aead::throw_backend_failure();
	}

	bytes trailing(static_cast<std::size_t>(final_size));
	std::copy_n(final_buffer.begin(), trailing.size(), trailing.begin());

	impl_->context.reset();
	impl_->phase = packet_stream_phase::finished;
	return trailing;
}

} // namespace securekit
