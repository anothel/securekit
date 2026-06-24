#include "securekit/aead.hpp"

#include "fuzz_utils.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
	const securekit::key256 key = securekit::fuzz::key_from_seed(0x10);
	const securekit::bytes packet = securekit::fuzz::raw_or_decoded_fixture(data, size);

	try
	{
		(void)securekit::decrypt(packet, key);
	}
	catch (const securekit::error &)
	{
	}

	return 0;
}
