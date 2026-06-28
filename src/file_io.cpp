#include "file_detail.hpp"

#include <array>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <istream>
#include <ostream>
#include <system_error>
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
#include "securekit/random.hpp"

namespace securekit::detail
{

OutputFile::OutputFile(std::filesystem::path path, std::FILE *file) : path_(std::move(path)), file_(file)
{
}

OutputFile::OutputFile(OutputFile &&other) noexcept : path_(std::move(other.path_)), file_(std::exchange(other.file_, nullptr))
{
}

OutputFile &OutputFile::operator=(OutputFile &&other) noexcept
{
	if (this != &other)
	{
		close_quietly();
		path_ = std::move(other.path_);
		file_ = std::exchange(other.file_, nullptr);
	}
	return *this;
}

OutputFile::~OutputFile()
{
	close_quietly();
}

const std::filesystem::path &OutputFile::path() const noexcept
{
	return path_;
}

void OutputFile::write(std::span<const std::byte> data)
{
	if (data.empty())
	{
		return;
	}
	const std::size_t written = std::fwrite(reinterpret_cast<const char *>(data.data()), 1, data.size(), file_);
	if (written != data.size())
	{
		throw_backend_failure("File write failed");
	}
}

void OutputFile::close()
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

void OutputFile::flush()
{
	if (file_ == nullptr)
	{
		return;
	}
	if (std::fflush(file_) != 0)
	{
		throw_backend_failure("File write failed");
	}
#if defined(_WIN32)
	const intptr_t os_handle = _get_osfhandle(_fileno(file_));
	if (os_handle == -1 || FlushFileBuffers(reinterpret_cast<HANDLE>(os_handle)) == 0)
	{
		throw_backend_failure("File write failed");
	}
#elif defined(__unix__) || defined(__APPLE__)
	if (::fsync(::fileno(file_)) != 0)
	{
		throw_backend_failure("File write failed");
	}
#endif
}

void OutputFile::discard() noexcept
{
	close_quietly();
}

void OutputFile::close_quietly() noexcept
{
	if (file_ != nullptr)
	{
		(void)std::fclose(file_);
		file_ = nullptr;
	}
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

void reject_trailing_data(std::istream &in)
{
	std::array<char, 1> trailing{};
	in.read(trailing.data(), static_cast<std::streamsize>(trailing.size()));
	if (in.gcount() != 0)
	{
		throw_invalid_packet();
	}
	if (in.bad() || (!in.eof() && in.fail()))
	{
		throw_backend_failure("File read failed");
	}
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

namespace
{

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
	const bytes token = random_bytes(16);
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

void flush_parent_directory(const std::filesystem::path &path)
{
#if defined(__unix__) || defined(__APPLE__)
	const std::filesystem::path parent = path.parent_path().empty() ? std::filesystem::path(".") : path.parent_path();
	const int fd = ::open(parent.c_str(), O_RDONLY);
	if (fd == -1)
	{
		throw_backend_failure("Directory open failed");
	}
	if (::fsync(fd) != 0)
	{
		(void)::close(fd);
		throw_backend_failure("Directory sync failed");
	}
	if (::close(fd) != 0)
	{
		throw_backend_failure("Directory sync failed");
	}
#else
	(void)path;
#endif
}

} // namespace

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
	if (MoveFileExW(temp_path.c_str(), output.c_str(), MOVEFILE_WRITE_THROUGH) != 0)
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
		flush_parent_directory(output);
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

} // namespace securekit::detail
