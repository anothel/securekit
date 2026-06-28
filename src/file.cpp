#include "securekit/file.hpp"

#include "file_detail.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <istream>
#include <limits>
#include <ostream>
#include <span>

#include "securekit/error.hpp"

#include "wipe.hpp"

namespace
{

using namespace securekit::detail;

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
		const EncryptedChunk encrypted =
		    encrypt_chunk(std::span<const std::byte>(buffer.data(), plaintext_size), file_key, nonce, chunk_aad);

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

		out.flush();
		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		out.discard();
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
		if (record.final_flag == kFinal)
		{
			reject_trailing_data(in);
		}

		const auto nonce = make_nonce(nonce_prefix, record.index);
		const securekit::bytes chunk_aad = make_chunk_aad(serialized_header, record, aad);
		securekit::bytes plaintext = decrypt_chunk(ciphertext, tag, file_key, nonce, chunk_aad);
		const securekit::internal::wipe_on_exit wipe_plaintext{std::span<std::byte>(plaintext)};
		write_all(out, plaintext);

		saw_final = record.final_flag == kFinal;
		if (expected_index == std::numeric_limits<std::uint32_t>::max() && !saw_final)
		{
			throw_invalid_packet();
		}
		++expected_index;
	}
}

} // namespace

namespace securekit
{

using namespace detail;

void seal_file(const std::filesystem::path &input, const std::filesystem::path &output, const key256 &key, std::span<const std::byte> aad)
{
	ensure_output_does_not_exist(output);
	const FileHeader header = make_header();
	key256 file_key = derive_file_key(key, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};
	seal_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

void seal_file(std::istream &input, std::ostream &output, const key256 &key, std::span<const std::byte> aad)
{
	const FileHeader header = make_header();
	key256 file_key = derive_file_key(key, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};
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
		key256 file_key = derive_file_key(key, header);
		const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};

		open_file_payload(in, out, header.serialized, header.nonce_prefix, file_key, aad);

		out.flush();
		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		out.discard();
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
	key256 file_key = derive_file_key(key, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};

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
	key256 file_key = derive_password_file_key(password, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};
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
	key256 file_key = derive_password_file_key(password, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};
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
		key256 file_key = derive_password_file_key(password, header);
		const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};

		open_file_payload(in, out, header.serialized, header.nonce_prefix, file_key, aad);

		out.flush();
		out.close();
		commit_temp_file(temp_path, output);
	}
	catch (...)
	{
		out.discard();
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
	key256 file_key = derive_password_file_key(password, header);
	const internal::wipe_on_exit wipe_file_key{std::span<std::byte>(file_key)};

	open_file_payload(input, output, header.serialized, header.nonce_prefix, file_key, aad);
}

} // namespace securekit
