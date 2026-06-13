#ifndef AMV_AEAD_HPP_
#define AMV_AEAD_HPP_

#include <cstddef>
#include <span>

#include "amv/export.hpp"
#include "amv/types.hpp"

namespace amv
{

AMV_API bytes encrypt(std::span<const std::byte> plaintext, const key256 &key, std::span<const std::byte> aad = {});
AMV_API bytes decrypt(std::span<const std::byte> packet, const key256 &key, std::span<const std::byte> aad = {});

} // namespace amv

#endif // AMV_AEAD_HPP_
