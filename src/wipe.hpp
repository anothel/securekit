#ifndef SECUREKIT_SRC_WIPE_HPP_
#define SECUREKIT_SRC_WIPE_HPP_

#include <openssl/crypto.h>

#include <array>
#include <cstddef>
#include <span>

#include "securekit/types.hpp"

namespace securekit::internal
{

inline void secure_wipe(std::span<std::byte> data) noexcept
{
	if (!data.empty())
	{
		OPENSSL_cleanse(data.data(), data.size());
	}
}

template <std::size_t Size>
inline void secure_wipe(std::array<std::byte, Size> &data) noexcept
{
	secure_wipe(std::span<std::byte>(data));
}

inline void secure_wipe(bytes &data) noexcept
{
	secure_wipe(std::span<std::byte>(data));
}

class wipe_on_exit
{
public:
	explicit wipe_on_exit(std::span<std::byte> data) noexcept : data_(data)
	{
	}

	wipe_on_exit(const wipe_on_exit &) = delete;
	wipe_on_exit &operator=(const wipe_on_exit &) = delete;

	~wipe_on_exit()
	{
		secure_wipe(data_);
	}

private:
	std::span<std::byte> data_;
};

} // namespace securekit::internal

#endif // SECUREKIT_SRC_WIPE_HPP_
