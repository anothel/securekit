#include "securekit/securekit.hpp"

#include <cstddef>
#include <iostream>
#include <string_view>

namespace
{

securekit::bytes bytes_from_text(std::string_view text)
{
	securekit::bytes out;
	out.reserve(text.size());
	for (unsigned char ch : text)
	{
		out.push_back(static_cast<std::byte>(ch));
	}
	return out;
}

} // namespace

int main()
{
	const securekit::bytes message = bytes_from_text("hello securekit");
	const securekit::bytes aad = bytes_from_text("example:v1");
	const securekit::key256 key = securekit::generate_key();

	const securekit::bytes packet = securekit::encrypt(message, key, aad);
	const securekit::bytes opened = securekit::decrypt(packet, key, aad);
	if (opened != message)
	{
		return 1;
	}

	std::cout << "sha256=" << securekit::hex_encode(securekit::sha256(message)) << '\n';
	return 0;
}
