#pragma once

#include "securekit/file.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <span>

namespace securekit::detail
{

inline constexpr std::uint32_t kChunkSize = 1024u * 1024u;
inline constexpr std::size_t kHeaderSize = 4 + 1 + 1 + 4 + 32 + 8;
inline constexpr std::size_t kPasswordHeaderSize = 4 + 1 + 1 + 1 + 1 + 4 + 32 + 8 + 4 + 4 + 4;
inline constexpr std::size_t kSaltSize = 32;
inline constexpr std::size_t kNoncePrefixSize = 8;
inline constexpr std::size_t kNonceSize = 12;
inline constexpr std::size_t kTagSize = 16;
inline constexpr std::size_t kRecordHeaderSize = 4 + 4 + 1;
inline constexpr std::byte kNotFinal{0x00};
inline constexpr std::byte kFinal{0x01};

struct FileHeader
{
	std::array<std::byte, kHeaderSize> serialized{};
	std::array<std::byte, kSaltSize> salt{};
	std::array<std::byte, kNoncePrefixSize> nonce_prefix{};
};

struct PasswordFileHeader
{
	std::array<std::byte, kPasswordHeaderSize> serialized{};
	std::array<std::byte, kSaltSize> salt{};
	std::array<std::byte, kNoncePrefixSize> nonce_prefix{};
	std::uint32_t scrypt_n = 32768u;
	std::uint32_t scrypt_r = 8u;
	std::uint32_t scrypt_p = 1u;
};

struct RecordHeader
{
	std::array<std::byte, kRecordHeaderSize> serialized{};
	std::uint32_t index{};
	std::uint32_t plaintext_size{};
	std::byte final_flag{};
};

struct EncryptedChunk
{
	bytes ciphertext;
	std::array<std::byte, kTagSize> tag{};
};

class OutputFile
{
public:
	OutputFile() = default;
	OutputFile(std::filesystem::path path, std::FILE *file);
	OutputFile(const OutputFile &) = delete;
	OutputFile &operator=(const OutputFile &) = delete;
	OutputFile(OutputFile &&other) noexcept;
	OutputFile &operator=(OutputFile &&other) noexcept;
	~OutputFile();

	[[nodiscard]] const std::filesystem::path &path() const noexcept;
	void write(std::span<const std::byte> data);
	void close();
	void flush();
	void discard() noexcept;

private:
	void close_quietly() noexcept;

	std::filesystem::path path_;
	std::FILE *file_ = nullptr;
};

[[noreturn]] void throw_backend_failure(const char *message);
[[noreturn]] void throw_invalid_packet();

FileHeader make_header();
FileHeader parse_header(std::span<const std::byte, kHeaderSize> data);
PasswordFileHeader make_password_header();
PasswordFileHeader parse_password_header(std::span<const std::byte, kPasswordHeaderSize> data);
key256 derive_file_key(const key256 &master_key, const FileHeader &header);
void require_non_empty_password(std::span<const std::byte> password);
key256 derive_password_file_key(std::span<const std::byte> password, const PasswordFileHeader &header);
std::array<std::byte, kNonceSize> make_nonce(std::span<const std::byte> nonce_prefix, std::uint32_t chunk_index);
RecordHeader make_record_header(std::uint32_t index, std::uint32_t plaintext_size, std::byte final_flag);
RecordHeader parse_record_header(std::span<const std::byte, kRecordHeaderSize> data);
bytes make_chunk_aad(
    std::span<const std::byte> serialized_header,
    const RecordHeader &record,
    std::span<const std::byte> caller_aad);
EncryptedChunk encrypt_chunk(
    std::span<const std::byte> plaintext,
    const key256 &file_key,
    std::span<const std::byte> nonce,
    std::span<const std::byte> aad);
bytes decrypt_chunk(
    std::span<const std::byte> ciphertext,
    std::span<const std::byte> tag,
    const key256 &file_key,
    std::span<const std::byte> nonce,
    std::span<const std::byte> aad);

void write_all(std::ostream &out, std::span<const std::byte> data);
void write_all(OutputFile &out, std::span<const std::byte> data);
bool read_exact(std::istream &in, std::byte *data, std::size_t size);
void reject_trailing_data(std::istream &in);
std::ifstream open_input(const std::filesystem::path &path);
void ensure_output_does_not_exist(const std::filesystem::path &output);
OutputFile open_unique_temp_output(const std::filesystem::path &output);
void remove_quietly(const std::filesystem::path &path);
void commit_temp_file(const std::filesystem::path &temp_path, const std::filesystem::path &output);

} // namespace securekit::detail
