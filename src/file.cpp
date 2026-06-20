#include "securekit/file.hpp"

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "securekit/error.hpp"
#include "securekit/hash.hpp"
#include "securekit/random.hpp"

namespace
{

constexpr std::array<std::byte, 4> kMagic{std::byte{'S'}, std::byte{'K'}, std::byte{'F'}, std::byte{'1'}};
constexpr std::array<std::byte, 4> kPasswordMagic{std::byte{'S'}, std::byte{'K'}, std::byte{'P'}, std::byte{'1'}};
constexpr std::byte kVersion{0x01};
constexpr std::byte kAlgorithm{0x01};
constexpr std::byte kPasswordCipherAes256Gcm{0x01};
constexpr std::byte kPasswordKdfScrypt{0x01};
constexpr std::byte kPasswordFlagsNone{0x00};
constexpr std::uint32_t kChunkSize = 1024u * 1024u;
constexpr std::size_t kHeaderSize = 4 + 1 + 1 + 4 + 32 + 8;
constexpr std::size_t kPasswordHeaderSize = 4 + 1 + 1 + 1 + 1 + 4 + 32 + 8 + 4 + 4 + 4;
constexpr std::size_t kSaltSize = 32;
constexpr std::size_t kNoncePrefixSize = 8;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kRecordHeaderSize = 4 + 4 + 1;
constexpr std::uint32_t kScryptN = 32768u;
constexpr std::uint32_t kScryptR = 8u;
constexpr std::uint32_t kScryptP = 1u;
constexpr std::uint64_t kScryptMaxMemory = 64ull * 1024ull * 1024ull;
constexpr std::byte kNotFinal{0x00};
constexpr std::byte kFinal{0x01};

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

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
	std::uint32_t scrypt_n = kScryptN;
	std::uint32_t scrypt_r = kScryptR;
	std::uint32_t scrypt_p = kScryptP;
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
	securekit::bytes ciphertext;
	std::array<std::byte, kTagSize> tag{};
};

[[noreturn]] void throw_backend_failure(const char *message);

class OutputFile
{
public:
	OutputFile() = default;

	OutputFile(std::filesystem::path path, std::FILE *file) : path_(std::move(path)), file_(file)
	{
	}

	OutputFile(const OutputFile &) = delete;
	OutputFile &operator=(const OutputFile &) = delete;

	OutputFile(OutputFile &&other) noexcept : path_(std::move(other.path_)), file_(std::exchange(other.file_, nullptr))
	{
	}

	OutputFile &operator=(OutputFile &&other) noexcept
	{
		if (this != &other)
		{
			close_quietly();
			path_ = std::move(other.path_);
			file_ = std::exchange(other.file_, nullptr);
		}
		return *this;
	}

	~OutputFile()
	{
		close_quietly();
	}

	[[nodiscard]] const std::filesystem::path &path() const noexcept
	{
		return path_;
	}

	void write(std::span<const std::byte> data)
	{
		if (data.empty())
		{
			return;
		}
		const std::size_t written =
		    std::fwrite(reinterpret_cast<const char *>(data.data()), 1, data.size(), file_);
		if (written != data.size())
		{
			throw_backend_failure("File write failed");
		}
	}

	void close()
	{
		if (file_ == nullptr)
		{
			return;
		}
		std::FILE *file = std::exchange(file_, nullptr);
		if (std::fclose(file) != 0)
		{
			throw_backend_failure("File write failed");
		}
	}

private:
	void close_quietly() noexcept
	{
		if (file_ != nullptr)
		{
			(void)std::fclose(file_);
			file_ = nullptr;
		}
	}

	std::filesystem::path path_;
	std::FILE *file_ = nullptr;
};

[[noreturn]] void throw_backend_failure(const char *message)
{
	throw securekit::error(securekit::error_code::backend_failure, message);
}

[[noreturn]] void throw_invalid_packet()
{
	throw securekit::error(securekit::error_code::invalid_packet, "Invalid file sealing packet");
}

[[noreturn]] void throw_authentication_failed()
{
	throw securekit::error(securekit::error_code::authentication_failed, "File authentication failed");
}

unsigned char *openssl_data(std::byte *data)
{
	return reinterpret_cast<unsigned char *>(data);
}

const unsigned char *openssl_data(std::span<const std::byte> data)
{
	return reinterpret_cast<const unsigned char *>(data.data());
}

void check_int_size(std::size_t size, const char *name)
{
	if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		throw securekit::error(securekit::error_code::invalid_input, std::string(name) + " exceeds OpenSSL update limit");
	}
}

int int_size(std::size_t size, const char *name)
{
	check_int_size(size, name);
	return static_cast<int>(size);
}

void store_u32_be(std::byte *output, std::uint32_t value)
{
	output[0] = static_cast<std::byte>((value >> 24) & 0xffu);
	output[1] = static_cast<std::byte>((value >> 16) & 0xffu);
	output[2] = static_cast<std::byte>((value >> 8) & 0xffu);
	output[3] = static_cast<std::byte>(value & 0xffu);
}

std::uint32_t read_u32_be(std::span<const std::byte, 4> input)
{
	return (static_cast<std::uint32_t>(input[0]) << 24) |
	       (static_cast<std::uint32_t>(input[1]) << 16) |
	       (static_cast<std::uint32_t>(input[2]) << 8) |
	       static_cast<std::uint32_t>(input[3]);
}

securekit::bytes bytes_from_literal(const char *text)
{
	securekit::bytes output;
	while (*text != '\0')
	{
		output.push_back(static_cast<std::byte>(static_cast<unsigned char>(*text)));
		++text;
	}
	return output;
}

FileHeader make_header()
{
	FileHeader header{};
	std::copy(kMagic.begin(), kMagic.end(), header.serialized.begin());
	header.serialized[4] = kVersion;
	header.serialized[5] = kAlgorithm;
	store_u32_be(header.serialized.data() + 6, kChunkSize);

	const securekit::bytes salt = securekit::random_bytes(kSaltSize);
	const securekit::bytes nonce_prefix = securekit::random_bytes(kNoncePrefixSize);
	std::copy(salt.begin(), salt.end(), header.salt.begin());
	std::copy(nonce_prefix.begin(), nonce_prefix.end(), header.nonce_prefix.begin());
	std::copy(header.salt.begin(), header.salt.end(), header.serialized.begin() + 10);
	std::copy(header.nonce_prefix.begin(), header.nonce_prefix.end(), header.serialized.begin() + 42);
	return header;
}

FileHeader parse_header(std::span<const std::byte, kHeaderSize> data)
{
	FileHeader header{};
	std::copy(data.begin(), data.end(), header.serialized.begin());
	if (!std::equal(kMagic.begin(), kMagic.end(), data.begin()) || data[4] != kVersion || data[5] != kAlgorithm)
	{
		throw_invalid_packet();
	}

	if (read_u32_be(data.template subspan<6, 4>()) != kChunkSize)
	{
		throw_invalid_packet();
	}

	std::copy(data.begin() + 10, data.begin() + 42, header.salt.begin());
	std::copy(data.begin() + 42, data.end(), header.nonce_prefix.begin());
	return header;
}

PasswordFileHeader make_password_header()
{
	PasswordFileHeader header{};
	std::copy(kPasswordMagic.begin(), kPasswordMagic.end(), header.serialized.begin());
	header.serialized[4] = kVersion;
	header.serialized[5] = kPasswordCipherAes256Gcm;
	header.serialized[6] = kPasswordKdfScrypt;
	header.serialized[7] = kPasswordFlagsNone;
	store_u32_be(header.serialized.data() + 8, kChunkSize);

	const securekit::bytes salt = securekit::random_bytes(kSaltSize);
	const securekit::bytes nonce_prefix = securekit::random_bytes(kNoncePrefixSize);
	std::copy(salt.begin(), salt.end(), header.salt.begin());
	std::copy(nonce_prefix.begin(), nonce_prefix.end(), header.nonce_prefix.begin());
	std::copy(header.salt.begin(), header.salt.end(), header.serialized.begin() + 12);
	std::copy(header.nonce_prefix.begin(), header.nonce_prefix.end(), header.serialized.begin() + 44);
	store_u32_be(header.serialized.data() + 52, header.scrypt_n);
	store_u32_be(header.serialized.data() + 56, header.scrypt_r);
	store_u32_be(header.serialized.data() + 60, header.scrypt_p);
	return header;
}

PasswordFileHeader parse_password_header(std::span<const std::byte, kPasswordHeaderSize> data)
{
	PasswordFileHeader header{};
	std::copy(data.begin(), data.end(), header.serialized.begin());
	if (!std::equal(kPasswordMagic.begin(), kPasswordMagic.end(), data.begin()) || data[4] != kVersion || data[5] != kPasswordCipherAes256Gcm ||
	    data[6] != kPasswordKdfScrypt || data[7] != kPasswordFlagsNone)
	{
		throw_invalid_packet();
	}

	if (read_u32_be(data.template subspan<8, 4>()) != kChunkSize)
	{
		throw_invalid_packet();
	}

	std::copy(data.begin() + 12, data.begin() + 44, header.salt.begin());
	std::copy(data.begin() + 44, data.begin() + 52, header.nonce_prefix.begin());
	header.scrypt_n = read_u32_be(data.template subspan<52, 4>());
	header.scrypt_r = read_u32_be(data.template subspan<56, 4>());
	header.scrypt_p = read_u32_be(data.template subspan<60, 4>());
	if (header.scrypt_n != kScryptN || header.scrypt_r != kScryptR || header.scrypt_p != kScryptP)
	{
		throw_invalid_packet();
	}

	return header;
}

securekit::key256 derive_file_key(const securekit::key256 &master_key, const FileHeader &header)
{
	const securekit::bytes info = bytes_from_literal("securekit file sealing v1");
	const securekit::bytes okm =
	    securekit::hkdf_sha256(std::span<const std::byte>(master_key), header.salt, info, securekit::key256{}.size());
	securekit::key256 file_key{};
	std::copy_n(okm.begin(), file_key.size(), file_key.begin());
	return file_key;
}

void require_non_empty_password(std::span<const std::byte> password)
{
	if (password.empty())
	{
		throw securekit::error(securekit::error_code::invalid_input, "password must not be empty");
	}
}

securekit::key256 derive_password_file_key(std::span<const std::byte> password, const PasswordFileHeader &header)
{
	require_non_empty_password(password);
	securekit::key256 file_key{};
	if (EVP_PBE_scrypt(
	        reinterpret_cast<const char *>(password.data()),
	        password.size(),
	        openssl_data(header.salt),
	        header.salt.size(),
	        header.scrypt_n,
	        header.scrypt_r,
	        header.scrypt_p,
	        kScryptMaxMemory,
	        openssl_data(file_key.data()),
	        file_key.size()) != 1)
	{
		throw_backend_failure("OpenSSL scrypt operation failed");
	}
	return file_key;
}

std::array<std::byte, kNonceSize> make_nonce(std::span<const std::byte> nonce_prefix, std::uint32_t chunk_index)
{
	std::array<std::byte, kNonceSize> nonce{};
	std::copy(nonce_prefix.begin(), nonce_prefix.end(), nonce.begin());
	store_u32_be(nonce.data() + kNoncePrefixSize, chunk_index);
	return nonce;
}

RecordHeader make_record_header(std::uint32_t index, std::uint32_t plaintext_size, std::byte final_flag)
{
	RecordHeader record{};
	record.index = index;
	record.plaintext_size = plaintext_size;
	record.final_flag = final_flag;
	store_u32_be(record.serialized.data(), index);
	store_u32_be(record.serialized.data() + 4, plaintext_size);
	record.serialized[8] = final_flag;
	return record;
}

RecordHeader parse_record_header(std::span<const std::byte, kRecordHeaderSize> data)
{
	RecordHeader record{};
	std::copy(data.begin(), data.end(), record.serialized.begin());
	record.index = read_u32_be(data.template subspan<0, 4>());
	record.plaintext_size = read_u32_be(data.template subspan<4, 4>());
	record.final_flag = data[8];
	if (record.final_flag != kNotFinal && record.final_flag != kFinal)
	{
		throw_invalid_packet();
	}
	if (record.plaintext_size > kChunkSize)
	{
		throw_invalid_packet();
	}
	return record;
}

securekit::bytes make_chunk_aad(std::span<const std::byte> serialized_header, const RecordHeader &record, std::span<const std::byte> caller_aad)
{
	securekit::bytes aad;
	aad.reserve(serialized_header.size() + record.serialized.size() + caller_aad.size());
	aad.insert(aad.end(), serialized_header.begin(), serialized_header.end());
	aad.insert(aad.end(), record.serialized.begin(), record.serialized.end());
	aad.insert(aad.end(), caller_aad.begin(), caller_aad.end());
	return aad;
}

CipherContext make_context()
{
	CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
	if (context == nullptr || EVP_aes_256_gcm() == nullptr)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}
	return context;
}

void update_aad(EVP_CIPHER_CTX *context, std::span<const std::byte> aad)
{
	const int aad_size = int_size(aad.size(), "AAD");
	if (aad_size == 0)
	{
		return;
	}

	int unused = 0;
	if (EVP_CipherUpdate(context, nullptr, &unused, openssl_data(aad), aad_size) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}
}

EncryptedChunk encrypt_chunk(std::span<const std::byte> plaintext, const securekit::key256 &file_key, std::span<const std::byte> nonce, std::span<const std::byte> aad)
{
	const int plaintext_size = int_size(plaintext.size(), "plaintext");
	EncryptedChunk output{};
	output.ciphertext.resize(plaintext.size());

	CipherContext context = make_context();
	const auto key_span = std::span<const std::byte>(file_key);
	const bool initialized =
	    EVP_EncryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
	    EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr) == 1 &&
	    EVP_EncryptInit_ex(context.get(), nullptr, nullptr, openssl_data(key_span), openssl_data(nonce)) == 1;
	if (!initialized)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	update_aad(context.get(), aad);

	int ciphertext_size = 0;
	if (plaintext_size > 0 &&
	    EVP_EncryptUpdate(context.get(), openssl_data(output.ciphertext.data()), &ciphertext_size, openssl_data(plaintext), plaintext_size) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	int final_size = 0;
	std::array<std::byte, 1> final_buffer{};
	std::byte *final_output = output.ciphertext.empty() ? final_buffer.data() : output.ciphertext.data() + ciphertext_size;
	if (EVP_EncryptFinal_ex(context.get(), openssl_data(final_output), &final_size) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}
	if (ciphertext_size + final_size != plaintext_size)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}
	if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(output.tag.size()), openssl_data(output.tag.data())) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	return output;
}

securekit::bytes decrypt_chunk(std::span<const std::byte> ciphertext, std::span<const std::byte> tag, const securekit::key256 &file_key, std::span<const std::byte> nonce, std::span<const std::byte> aad)
{
	const int ciphertext_size = int_size(ciphertext.size(), "ciphertext");
	securekit::bytes plaintext(ciphertext.size());

	CipherContext context = make_context();
	const auto key_span = std::span<const std::byte>(file_key);
	const bool initialized =
	    EVP_DecryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
	    EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce.size()), nullptr) == 1 &&
	    EVP_DecryptInit_ex(context.get(), nullptr, nullptr, openssl_data(key_span), openssl_data(nonce)) == 1;
	if (!initialized)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	update_aad(context.get(), aad);

	int plaintext_size = 0;
	if (ciphertext_size > 0 && EVP_DecryptUpdate(context.get(), openssl_data(plaintext.data()), &plaintext_size, openssl_data(ciphertext), ciphertext_size) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	std::array<std::byte, kTagSize> tag_copy{};
	std::copy(tag.begin(), tag.end(), tag_copy.begin());
	if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag_copy.size()), openssl_data(tag_copy.data())) != 1)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	std::array<std::byte, 1> final_buffer{};
	std::byte *final_output = plaintext.empty() ? final_buffer.data() : plaintext.data() + plaintext_size;
	int final_size = 0;
	if (EVP_DecryptFinal_ex(context.get(), openssl_data(final_output), &final_size) != 1)
	{
		std::fill(plaintext.begin(), plaintext.end(), std::byte{0});
		throw_authentication_failed();
	}
	if (plaintext_size + final_size != ciphertext_size)
	{
		throw_backend_failure("OpenSSL AES-256-GCM operation failed");
	}

	return plaintext;
}

void write_all(std::ostream &out, std::span<const std::byte> data)
{
	out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
	if (!out)
	{
		throw_backend_failure("File write failed");
	}
}

void write_all(OutputFile &out, std::span<const std::byte> data)
{
	out.write(data);
}

bool read_exact(std::istream &in, std::byte *data, std::size_t size)
{
	in.read(reinterpret_cast<char *>(data), static_cast<std::streamsize>(size));
	if (static_cast<std::size_t>(in.gcount()) == size)
	{
		return true;
	}
	if (in.bad() || (!in.eof() && in.fail()))
	{
		throw_backend_failure("File read failed");
	}
	return false;
}

std::ifstream open_input(const std::filesystem::path &path)
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		throw_backend_failure("File open failed");
	}
	return input;
}

enum class ExclusiveOpenResult
{
	opened,
	already_exists,
	failed,
};

ExclusiveOpenResult try_open_output_exclusive(const std::filesystem::path &path, OutputFile &output)
{
#if defined(_WIN32)
	int fd = -1;
	const errno_t result =
	    _wsopen_s(&fd, path.c_str(), _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (result == EEXIST)
	{
		return ExclusiveOpenResult::already_exists;
	}
	if (result != 0)
	{
		return ExclusiveOpenResult::failed;
	}

	std::FILE *file = _fdopen(fd, "wb");
	if (file == nullptr)
	{
		_close(fd);
		return ExclusiveOpenResult::failed;
	}
	output = OutputFile(path, file);
	return ExclusiveOpenResult::opened;
#else
	const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd == -1)
	{
		if (errno == EEXIST)
		{
			return ExclusiveOpenResult::already_exists;
		}
		return ExclusiveOpenResult::failed;
	}

	std::FILE *file = ::fdopen(fd, "wb");
	if (file == nullptr)
	{
		(void)::close(fd);
		return ExclusiveOpenResult::failed;
	}
	output = OutputFile(path, file);
	return ExclusiveOpenResult::opened;
#endif
}

std::filesystem::path make_unique_temp_path(const std::filesystem::path &output)
{
	constexpr std::array<char, 16> hex_digits{
	    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	const securekit::bytes token = securekit::random_bytes(16);
	std::string suffix;
	suffix.reserve(45);
	suffix += ".securekit.";
	for (const std::byte value : token)
	{
		const auto byte_value = static_cast<unsigned char>(value);
		suffix.push_back(hex_digits[(byte_value >> 4) & 0x0f]);
		suffix.push_back(hex_digits[byte_value & 0x0f]);
	}
	suffix += ".tmp";

	std::filesystem::path temp = output;
	temp += suffix;
	return temp;
}

void ensure_output_does_not_exist(const std::filesystem::path &output)
{
	std::error_code ec;
	const bool exists = std::filesystem::exists(output, ec);
	if (ec)
	{
		throw_backend_failure("File status check failed");
	}
	if (exists)
	{
		throw securekit::error(securekit::error_code::invalid_input, "Output file already exists");
	}
}

OutputFile open_unique_temp_output(const std::filesystem::path &output)
{
	constexpr int kMaxAttempts = 32;
	for (int attempt = 0; attempt < kMaxAttempts; ++attempt)
	{
		OutputFile temp;
		const ExclusiveOpenResult result = try_open_output_exclusive(make_unique_temp_path(output), temp);
		if (result == ExclusiveOpenResult::opened)
		{
			return temp;
		}
		if (result == ExclusiveOpenResult::failed)
		{
			throw_backend_failure("File open failed");
		}
	}
	throw_backend_failure("Temporary file creation failed");
}

void remove_quietly(const std::filesystem::path &path)
{
	std::error_code ec;
	(void)std::filesystem::remove(path, ec);
}

void commit_temp_file(const std::filesystem::path &temp_path, const std::filesystem::path &output)
{
#if defined(_WIN32)
	if (MoveFileExW(temp_path.c_str(), output.c_str(), 0) != 0)
	{
		return;
	}
	const DWORD error = GetLastError();
	if (error == ERROR_ALREADY_EXISTS || error == ERROR_FILE_EXISTS)
	{
		throw securekit::error(securekit::error_code::invalid_input, "Output file already exists");
	}
	throw_backend_failure("File rename failed");
#elif defined(__unix__) || defined(__APPLE__)
	if (::link(temp_path.c_str(), output.c_str()) == 0)
	{
		remove_quietly(temp_path);
		return;
	}
	if (errno == EEXIST)
	{
		throw securekit::error(securekit::error_code::invalid_input, "Output file already exists");
	}
	throw_backend_failure("File rename failed");
#else
	std::error_code ec;
	const bool exists = std::filesystem::exists(output, ec);
	if (ec)
	{
		throw_backend_failure("File status check failed");
	}
	if (exists)
	{
		throw securekit::error(securekit::error_code::invalid_input, "Output file already exists");
	}
	std::filesystem::rename(temp_path, output, ec);
	if (ec)
	{
		throw_backend_failure("File rename failed");
	}
#endif
}

template <typename Output>
void seal_stream_payload(
    std::istream &in,
    Output &out,
    std::span<const std::byte> serialized_header,
    std::span<const std::byte> nonce_prefix,
    const securekit::key256 &file_key,
    std::span<const std::byte> aad)
{
	write_all(out, serialized_header);

	securekit::bytes buffer(kChunkSize);
	std::uint32_t chunk_index = 0;
	bool wrote_final = false;
	while (!wrote_final)
	{
		in.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
		const std::streamsize read_count = in.gcount();
		if (read_count < 0)
		{
			throw_backend_failure("File read failed");
		}
		if (!in.eof() && in.fail())
		{
			throw_backend_failure("File read failed");
		}

		const auto plaintext_size = static_cast<std::uint32_t>(read_count);
		bool is_final = plaintext_size < kChunkSize;
		if (!is_final && in.peek() == std::char_traits<char>::eof())
		{
			if (in.bad())
			{
				throw_backend_failure("File read failed");
			}
			is_final = true;
		}
		const RecordHeader record = make_record_header(chunk_index, plaintext_size, is_final ? kFinal : kNotFinal);
		const auto nonce = make_nonce(nonce_prefix, chunk_index);
		const securekit::bytes chunk_aad = make_chunk_aad(serialized_header, record, aad);
		const EncryptedChunk encrypted = encrypt_chunk(std::span<const std::byte>(buffer.data(), plaintext_size), file_key, nonce, chunk_aad);

		write_all(out, record.serialized);
		write_all(out, encrypted.ciphertext);
		write_all(out, encrypted.tag);

		wrote_final = is_final;
		if (chunk_index == std::numeric_limits<std::uint32_t>::max() && !wrote_final)
		{
			throw securekit::error(securekit::error_code::invalid_input, "file is too large to seal");
		}
		++chunk_index;
	}
}

void seal_file_payload(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    std::span<const std::byte> serialized_header,
    std::span<const std::byte> nonce_prefix,
    const securekit::key256 &file_key,
    std::span<const std::byte> aad)
{
	OutputFile out = open_unique_temp_output(output);
	const std::filesystem::path temp_path = out.path();
	try
	{
		std::ifstream in = open_input(input);

		seal_stream_payload(in, out, serialized_header, nonce_prefix, file_key, aad);

		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		remove_quietly(temp_path);
		throw;
	}
}

template <typename Output>
void open_file_payload(
    std::istream &in,
    Output &out,
    std::span<const std::byte> serialized_header,
    std::span<const std::byte> nonce_prefix,
    const securekit::key256 &file_key,
    std::span<const std::byte> aad)
{
	std::uint32_t expected_index = 0;
	bool saw_final = false;
	while (!saw_final)
	{
		std::array<std::byte, kRecordHeaderSize> record_bytes{};
		if (!read_exact(in, record_bytes.data(), record_bytes.size()))
		{
			throw_invalid_packet();
		}
		const RecordHeader record = parse_record_header(record_bytes);
		if (record.index != expected_index)
		{
			throw_invalid_packet();
		}
		if (record.final_flag == kNotFinal && record.plaintext_size != kChunkSize)
		{
			throw_invalid_packet();
		}

		securekit::bytes ciphertext(record.plaintext_size);
		if (!read_exact(in, ciphertext.data(), ciphertext.size()))
		{
			throw_invalid_packet();
		}
		std::array<std::byte, kTagSize> tag{};
		if (!read_exact(in, tag.data(), tag.size()))
		{
			throw_invalid_packet();
		}

		const auto nonce = make_nonce(nonce_prefix, record.index);
		const securekit::bytes chunk_aad = make_chunk_aad(serialized_header, record, aad);
		const securekit::bytes plaintext = decrypt_chunk(ciphertext, tag, file_key, nonce, chunk_aad);
		write_all(out, plaintext);

		saw_final = record.final_flag == kFinal;
		if (expected_index == std::numeric_limits<std::uint32_t>::max() && !saw_final)
		{
			throw_invalid_packet();
		}
		++expected_index;
	}

	std::array<char, 1> trailing{};
	in.read(trailing.data(), static_cast<std::streamsize>(trailing.size()));
	if (in.gcount() != 0)
	{
		throw_invalid_packet();
	}
}

} // namespace

namespace securekit
{

void seal_file(const std::filesystem::path &input, const std::filesystem::path &output, const key256 &key, std::span<const std::byte> aad)
{
	ensure_output_does_not_exist(output);
	const FileHeader header = make_header();
	const key256 file_key = derive_file_key(key, header);
	seal_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void seal_file(std::istream &input, std::ostream &output, const key256 &key, std::span<const std::byte> aad)
{
	const FileHeader header = make_header();
	const key256 file_key = derive_file_key(key, header);
	seal_stream_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void open_file(const std::filesystem::path &input, const std::filesystem::path &output, const key256 &key, std::span<const std::byte> aad)
{
	ensure_output_does_not_exist(output);
	OutputFile out = open_unique_temp_output(output);
	const std::filesystem::path temp_path = out.path();
	try
	{
		std::ifstream in = open_input(input);

		std::array<std::byte, kHeaderSize> header_bytes{};
		if (!read_exact(in, header_bytes.data(), header_bytes.size()))
		{
			throw_invalid_packet();
		}
		const FileHeader header = parse_header(header_bytes);
		const key256 file_key = derive_file_key(key, header);

		open_file_payload(in, out, header.serialized, header.nonce_prefix, file_key, aad);

		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		remove_quietly(temp_path);
		throw;
	}
}

void open_file(std::istream &input, std::ostream &output, const key256 &key, std::span<const std::byte> aad)
{
	std::array<std::byte, kHeaderSize> header_bytes{};
	if (!read_exact(input, header_bytes.data(), header_bytes.size()))
	{
		throw_invalid_packet();
	}
	const FileHeader header = parse_header(header_bytes);
	const key256 file_key = derive_file_key(key, header);

	open_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void seal_file_with_password(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad)
{
	require_non_empty_password(password);
	ensure_output_does_not_exist(output);
	const PasswordFileHeader header = make_password_header();
	const key256 file_key = derive_password_file_key(password, header);
	seal_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void seal_file_with_password(
    std::istream &input,
    std::ostream &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad)
{
	require_non_empty_password(password);
	const PasswordFileHeader header = make_password_header();
	const key256 file_key = derive_password_file_key(password, header);
	seal_stream_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void open_file_with_password(
    const std::filesystem::path &input,
    const std::filesystem::path &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad)
{
	require_non_empty_password(password);
	ensure_output_does_not_exist(output);
	OutputFile out = open_unique_temp_output(output);
	const std::filesystem::path temp_path = out.path();
	try
	{
		std::ifstream in = open_input(input);

		std::array<std::byte, kPasswordHeaderSize> header_bytes{};
		if (!read_exact(in, header_bytes.data(), header_bytes.size()))
		{
			throw_invalid_packet();
		}
		const PasswordFileHeader header = parse_password_header(header_bytes);
		const key256 file_key = derive_password_file_key(password, header);

		open_file_payload(in, out, header.serialized, header.nonce_prefix, file_key, aad);

		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		remove_quietly(temp_path);
		throw;
	}
}

void open_file_with_password(
    std::istream &input,
    std::ostream &output,
    std::span<const std::byte> password,
    std::span<const std::byte> aad)
{
	require_non_empty_password(password);
	std::array<std::byte, kPasswordHeaderSize> header_bytes{};
	if (!read_exact(input, header_bytes.data(), header_bytes.size()))
	{
		throw_invalid_packet();
	}
	const PasswordFileHeader header = parse_password_header(header_bytes);
	const key256 file_key = derive_password_file_key(password, header);

	open_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

} // namespace securekit
