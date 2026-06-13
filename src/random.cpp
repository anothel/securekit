#include "securekit/random.hpp"

#include <algorithm>
#include <limits>

#include <openssl/rand.h>

#include "securekit/error.hpp"

namespace
{

void fill_random(unsigned char *output, std::size_t size)
{
	while (size > 0)
	{
		const std::size_t chunk_size = std::min<std::size_t>(size, std::numeric_limits<int>::max());
		const auto chunk = static_cast<int>(chunk_size);
		if (RAND_bytes(output, chunk) != 1)
		{
			throw securekit::error(securekit::error_code::backend_failure, "OpenSSL RAND_bytes failed");
		}
		output += chunk;
		size -= static_cast<std::size_t>(chunk);
	}
}

} // namespace

namespace securekit
{

bytes random_bytes(std::size_t size)
{
	bytes output(size);
	if (output.empty())
	{
		return output;
	}

	fill_random(reinterpret_cast<unsigned char *>(output.data()), output.size());
	return output;
}

key256 generate_key()
{
	key256 key{};
	auto *key_data = reinterpret_cast<unsigned char *>(key.data());
	const auto key_size = static_cast<int>(key.size());
	if (RAND_priv_bytes(key_data, key_size) != 1)
	{
		throw error(error_code::backend_failure, "OpenSSL RAND_priv_bytes failed");
	}
	return key;
}

} // namespace securekit
