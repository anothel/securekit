#include "securekit/key_wrap.hpp"

#include <algorithm>
#include <span>

#include "securekit/aead.hpp"
#include "securekit/error.hpp"

namespace securekit
{

bytes wrap_key(const key256 &key_to_wrap, const key256 &wrapping_key, std::span<const std::byte> aad)
{
	return encrypt(std::span<const std::byte>(key_to_wrap), wrapping_key, aad);
}

key256 unwrap_key(std::span<const std::byte> packet, const key256 &wrapping_key, std::span<const std::byte> aad)
{
	const bytes key_bytes = decrypt(packet, wrapping_key, aad);
	if (key_bytes.size() != key256{}.size())
	{
		throw error(error_code::invalid_packet, "Invalid wrapped key packet");
	}

	key256 key{};
	std::copy_n(key_bytes.begin(), key.size(), key.begin());
	return key;
}

} // namespace securekit
