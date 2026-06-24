#include "securekit/file.hpp"

#include <sstream>

#include "fuzz_utils.hpp"

namespace
{

void try_open_file(const securekit::bytes &data)
{
	const std::string input_bytes(reinterpret_cast<const char *>(data.data()), data.size());
	std::istringstream input(input_bytes, std::ios::in | std::ios::binary);
	std::ostringstream output(std::ios::out | std::ios::binary);
	const securekit::key256 key = securekit::fuzz::key_from_seed(0x20);

	try
	{
		securekit::open_file(input, output, key);
	}
	catch (const securekit::error &)
	{
	}
}

void try_open_password_file(const securekit::bytes &data)
{
	if (data.size() >= 4 && data[0] == std::byte{'S'} && data[1] == std::byte{'K'} && data[2] == std::byte{'P'} &&
	    data[3] == std::byte{'1'} && data.size() >= 64)
	{
		return;
	}

	const std::string input_bytes(reinterpret_cast<const char *>(data.data()), data.size());
	std::istringstream input(input_bytes, std::ios::in | std::ios::binary);
	std::ostringstream output(std::ios::out | std::ios::binary);
	const securekit::bytes password{std::byte{'f'}, std::byte{'u'}, std::byte{'z'}, std::byte{'z'}};

	try
	{
		securekit::open_file_with_password(input, output, password);
	}
	catch (const securekit::error &)
	{
	}
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size)
{
	const securekit::bytes payload = securekit::fuzz::raw_or_decoded_fixture(data, size);
	try_open_file(payload);
	try_open_password_file(payload);
	return 0;
}
