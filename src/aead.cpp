#include "securekit/aead.hpp"

#include <algorithm>
#include <cstddef>
#include <span>

#include "securekit/error.hpp"
#include "securekit/packet_stream.hpp"

#include "aead_internal.hpp"

namespace securekit
{

bytes encrypt(std::span<const std::byte> plaintext, const key256 &key, std::span<const std::byte> aad)
{
	packet_encryptor encryptor(key, aad);
	bytes packet = encryptor.begin();
	bytes ciphertext = encryptor.update(plaintext);
	bytes tag = encryptor.finalize();
	packet.insert(packet.end(), ciphertext.begin(), ciphertext.end());
	packet.insert(packet.end(), tag.begin(), tag.end());
	return packet;
}

bytes decrypt(std::span<const std::byte> packet, const key256 &key, std::span<const std::byte> aad)
{
	internal_aead::require_valid_packet(packet);

	const std::span<const std::byte> prefix = packet.first(internal_aead::kPrefixSize);
	const std::span<const std::byte> ciphertext =
	    packet.subspan(internal_aead::kPrefixSize, packet.size() - internal_aead::kOverhead);
	const std::span<const std::byte> tag = packet.last(internal_aead::kTagSize);

	packet_decryptor decryptor(key, aad);
	decryptor.begin(prefix);
	bytes plaintext = decryptor.update(ciphertext);
	bytes trailing;
	try
	{
		trailing = decryptor.finalize(tag);
	}
	catch (const error &)
	{
		std::fill(plaintext.begin(), plaintext.end(), std::byte{0});
		throw;
	}
	plaintext.insert(plaintext.end(), trailing.begin(), trailing.end());
	return plaintext;
}

} // namespace securekit
