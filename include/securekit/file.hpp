#ifndef SECUREKIT_FILE_HPP_
#define SECUREKIT_FILE_HPP_

#include <cstddef>
#include <filesystem>
#include <span>

#include "securekit/export.hpp"
#include "securekit/types.hpp"

namespace securekit
{

SECUREKIT_API void seal_file(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    const key256 &key,
    std::span<const std::byte> aad = {});

SECUREKIT_API void open_file(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    const key256 &key,
    std::span<const std::byte> aad = {});

} // namespace securekit

#endif // SECUREKIT_FILE_HPP_
