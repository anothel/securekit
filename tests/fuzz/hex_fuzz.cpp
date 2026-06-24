#include "securekit/hex.hpp"

#include "fuzz_utils.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
	const securekit::bytes bytes = securekit::fuzz::bytes_from_data(data, size);
	(void)securekit::hex_encode(bytes);

	try
	{
		(void)securekit::hex_decode(securekit::fuzz::string_from_data(data, size));
	}
	catch (const securekit::error &)
	{
	}

	return 0;
}
