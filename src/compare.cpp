#include "securekit/compare.hpp"

#include <algorithm>
#include <cstddef>

namespace securekit
{

bool constant_time_equal(std::span<const std::byte> left, std::span<const std::byte> right)
{
	std::size_t diff = left.size() ^ right.size();
	const std::size_t common_size = std::min(left.size(), right.size());

	for (std::size_t index = 0; index < common_size; ++index)
	{
		diff |= static_cast<std::size_t>(std::to_integer<unsigned char>(left[index]) ^ std::to_integer<unsigned char>(right[index]));
	}

	return diff == 0;
}

} // namespace securekit
