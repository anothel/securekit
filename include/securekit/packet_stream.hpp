#ifndef SECUREKIT_PACKET_STREAM_HPP_
#define SECUREKIT_PACKET_STREAM_HPP_

#include <cstddef>
#include <memory>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

class packet_encryptor
{
public:
	SECUREKIT_API explicit packet_encryptor(const key256 &key, std::span<const std::byte> aad = {});
	SECUREKIT_API ~packet_encryptor();

	SECUREKIT_API packet_encryptor(packet_encryptor &&other) noexcept;
	SECUREKIT_API packet_encryptor &operator=(packet_encryptor &&other) noexcept;

	packet_encryptor(const packet_encryptor &) = delete;
	packet_encryptor &operator=(const packet_encryptor &) = delete;

	SECUREKIT_API bytes begin();
	SECUREKIT_API bytes update(std::span<const std::byte> plaintext);
	SECUREKIT_API bytes finalize();

private:
	struct impl;
	std::unique_ptr<impl> impl_;
};

class packet_decryptor
{
public:
	SECUREKIT_API explicit packet_decryptor(const key256 &key, std::span<const std::byte> aad = {});
	SECUREKIT_API ~packet_decryptor();

	SECUREKIT_API packet_decryptor(packet_decryptor &&other) noexcept;
	SECUREKIT_API packet_decryptor &operator=(packet_decryptor &&other) noexcept;

	packet_decryptor(const packet_decryptor &) = delete;
	packet_decryptor &operator=(const packet_decryptor &) = delete;

	SECUREKIT_API void begin(std::span<const std::byte> packet_prefix);
	SECUREKIT_API bytes update(std::span<const std::byte> ciphertext);
	SECUREKIT_API bytes finalize(std::span<const std::byte> tag);

private:
	struct impl;
	std::unique_ptr<impl> impl_;
};

} // namespace securekit

#endif // SECUREKIT_PACKET_STREAM_HPP_
