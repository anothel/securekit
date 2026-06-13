#include "securekit/file.hpp"

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <span>
#include <string>

#include "securekit/error.hpp"
#include "securekit/hash.hpp"
#include "securekit/random.hpp"

namespace
{

constexpr std::array<std::byte, 4> kMagic{std::byte{'S'}, std::byte{'K'}, std::byte{'F'}, std::byte{'1'}};
constexpr std::byte kVersion{0x01};
constexpr std::byte kAlgorithm{0x01};
constexpr std::uint32_t kChunkSize = 1024u * 1024u;
constexpr std::size_t kHeaderSize = 4 + 1 + 1 + 4 + 32 + 8;
constexpr std::size_t kSaltSize = 32;
constexpr std::size_t kNoncePrefixSize = 8;
constexpr std::size_t kNonceSize = 12;
constexpr std::size_t kTagSize = 16;
constexpr std::size_t kRecordHeaderSize = 4 + 4 + 1;
constexpr std::byte kNotFinal{0x00};
constexpr std::byte kFinal{0x01};

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

struct FileHeader
{
	std::array<std::byte, kHeaderSize> serialized{};
	std::array<std::byte, kSaltSize> salt{};
	std::array<std::byte, kNoncePrefixSize> nonce_prefix{};
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

securekit::key256 derive_file_key(const securekit::key256 &master_key, const FileHeader &header)
{
	const securekit::bytes info = bytes_from_literal("securekit file sealing v1");
	const securekit::bytes okm =
	    securekit::hkdf_sha256(std::span<const std::byte>(master_key), header.salt, info, securekit::key256{}.size());
	securekit::key256 file_key{};
	std::copy_n(okm.begin(), file_key.size(), file_key.begin());
	return file_key;
}

std::array<std::byte, kNonceSize> make_nonce(const FileHeader &header, std::uint32_t chunk_index)
{
	std::array<std::byte, kNonceSize> nonce{};
	std::copy(header.nonce_prefix.begin(), header.nonce_prefix.end(), nonce.begin());
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

securekit::bytes make_chunk_aad(const FileHeader &header, const RecordHeader &record, std::span<const std::byte> caller_aad)
{
	securekit::bytes aad;
	aad.reserve(header.serialized.size() + record.serialized.size() + caller_aad.size());
	aad.insert(aad.end(), header.serialized.begin(), header.serialized.end());
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

bool read_exact(std::istream &in, std::byte *data, std::size_t size)
{
	in.read(reinterpret_cast<char *>(data), static_cast<std::streamsize>(size));
	return static_cast<std::size_t>(in.gcount()) == size;
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

std::ofstream open_output(const std::filesystem::path &path)
{
	std::ofstream output(path, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		throw_backend_failure("File open failed");
	}
	return output;
}

std::filesystem::path temp_path_for(const std::filesystem::path &output)
{
	std::filesystem::path temp = output;
	temp += ".securekit.tmp";
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

void remove_quietly(const std::filesystem::path &path)
{
	std::error_code ec;
	(void)std::filesystem::remove(path, ec);
}

void rename_temp_file(const std::filesystem::path &temp_path, const std::filesystem::path &output)
{
	std::error_code ec;
	std::filesystem::rename(temp_path, output, ec);
	if (ec)
	{
		throw_backend_failure("File rename failed");
	}
}

} // namespace

namespace securekit
{

void seal_file(const std::filesystem::path &input, const std::filesystem::path &output, const key256 &key, std::span<const std::byte> aad)
{
	ensure_output_does_not_exist(output);
	const std::filesystem::path temp_path = temp_path_for(output);
	try
	{
		std::ifstream in = open_input(input);
		std::ofstream out = open_output(temp_path);

		const FileHeader header = make_header();
		const key256 file_key = derive_file_key(key, header);
		write_all(out, header.serialized);

		bytes buffer(kChunkSize);
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
			const bool is_final = plaintext_size < kChunkSize;
			const RecordHeader record = make_record_header(chunk_index, plaintext_size, is_final ? kFinal : kNotFinal);
			const auto nonce = make_nonce(header, chunk_index);
			const bytes chunk_aad = make_chunk_aad(header, record, aad);
			const EncryptedChunk encrypted = encrypt_chunk(std::span<const std::byte>(buffer.data(), plaintext_size), file_key, nonce, chunk_aad);

			write_all(out, record.serialized);
			write_all(out, encrypted.ciphertext);
			write_all(out, encrypted.tag);

			wrote_final = is_final;
			if (chunk_index == std::numeric_limits<std::uint32_t>::max() && !wrote_final)
			{
				throw error(error_code::invalid_input, "file is too large to seal");
			}
			++chunk_index;
		}

		out.close();
		if (!out)
		{
			throw_backend_failure("File write failed");
		}
		rename_temp_file(temp_path, output);
	}
	catch (...)
	{
		remove_quietly(temp_path);
		throw;
	}
}

void open_file(const std::filesystem::path &input, const std::filesystem::path &output, const key256 &key, std::span<const std::byte> aad)
{
	ensure_output_does_not_exist(output);
	const std::filesystem::path temp_path = temp_path_for(output);
	try
	{
		std::ifstream in = open_input(input);
		std::ofstream out = open_output(temp_path);

		std::array<std::byte, kHeaderSize> header_bytes{};
		if (!read_exact(in, header_bytes.data(), header_bytes.size()))
		{
			throw_invalid_packet();
		}
		const FileHeader header = parse_header(header_bytes);
		const key256 file_key = derive_file_key(key, header);

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

			bytes ciphertext(record.plaintext_size);
			if (!read_exact(in, ciphertext.data(), ciphertext.size()))
			{
				throw_invalid_packet();
			}
			std::array<std::byte, kTagSize> tag{};
			if (!read_exact(in, tag.data(), tag.size()))
			{
				throw_invalid_packet();
			}

			const auto nonce = make_nonce(header, record.index);
			const bytes chunk_aad = make_chunk_aad(header, record, aad);
			const bytes plaintext = decrypt_chunk(ciphertext, tag, file_key, nonce, chunk_aad);
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

		out.close();
		if (!out)
		{
			throw_backend_failure("File write failed");
		}
		rename_temp_file(temp_path, output);
	}
	catch (...)
	{
		remove_quietly(temp_path);
		throw;
	}
}

} // namespace securekit
