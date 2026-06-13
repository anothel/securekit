#include <cstddef>
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

} // namespace

int main()
{
	const auto plaintext = ascii_bytes("SecureKit consumer sample plaintext");
	const auto aad = ascii_bytes("SecureKit consumer sample aad");
	const auto key = securekit::generate_key();

	const auto packet = securekit::encrypt(plaintext, key, aad);
	const auto roundtrip = securekit::decrypt(packet, key, aad);

	return roundtrip == plaintext ? 0 : 1;
}
