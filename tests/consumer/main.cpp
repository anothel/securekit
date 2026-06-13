#include <cstddef>
#include <string_view>

#include "amv/amv.hpp"

namespace
{

amv::bytes ascii_bytes(std::string_view text)
{
	amv::bytes result;
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
	const auto plaintext = ascii_bytes("AMV consumer sample plaintext");
	const auto aad = ascii_bytes("AMV consumer sample aad");
	const auto key = amv::generate_key();

	const auto packet = amv::encrypt(plaintext, key, aad);
	const auto roundtrip = amv::decrypt(packet, key, aad);

	return roundtrip == plaintext ? 0 : 1;
}
