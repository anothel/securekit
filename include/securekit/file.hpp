#ifndef SECUREKIT_FILE_HPP_
#define SECUREKIT_FILE_HPP_

#include <cstddef>
#include <filesystem>
#include <iosfwd>
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

SECUREKIT_API void seal_file(
    std::istream &input,
    std::ostream &output,
    const key256 &key,
    std::span<const std::byte> aad = {});

// Path overload refuses an existing output path, writes a SecureKit-owned
// temporary file, and commits it only after authentication succeeds.
SECUREKIT_API void open_file(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    const key256 &key,
    std::span<const std::byte> aad = {});

// Stream overload writes to caller-owned output; callers own rollback or
// discard policy if this function throws after writing bytes.
SECUREKIT_API void open_file(
    std::istream &input,
    std::ostream &output,
    const key256 &key,
    std::span<const std::byte> aad = {});

// Password bytes are used exactly as supplied: no trimming, normalization,
// prompting, encoding conversion, or environment lookup.
SECUREKIT_API void seal_file_with_password(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad = {});

SECUREKIT_API void seal_file_with_password(
    std::istream &input,
    std::ostream &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad = {});

// Path overload has the same no-overwrite and temporary-file commit behavior
// as open_file.
SECUREKIT_API void open_file_with_password(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad = {});

// Stream overload writes to caller-owned output; callers own rollback or
// discard policy if this function throws after writing bytes.
SECUREKIT_API void open_file_with_password(
    std::istream &input,
    std::ostream &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad = {});

} // namespace securekit

#endif // SECUREKIT_FILE_HPP_
