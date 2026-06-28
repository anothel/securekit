#include "securekit/securekit.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace
{

using steady_clock_t = std::chrono::steady_clock;

struct measurement
{
	std::string name;
	std::size_t iterations;
	std::size_t bytes;
	double seconds;
	std::uint64_t guard;
};

class temp_directory
{
public:
	explicit temp_directory(std::filesystem::path path) : path_(std::move(path))
	{
		std::filesystem::create_directory(path_);
	}

	temp_directory(const temp_directory &) = delete;
	temp_directory &operator=(const temp_directory &) = delete;

	~temp_directory()
	{
		std::error_code ignored;
		std::filesystem::remove_all(path_, ignored);
	}

	[[nodiscard]] const std::filesystem::path &path() const
	{
		return path_;
	}

private:
	std::filesystem::path path_;
};

securekit::bytes make_bytes(std::size_t size)
{
	securekit::bytes bytes;
	bytes.reserve(size);
	for (std::size_t index = 0; index < size; ++index)
	{
		bytes.push_back(static_cast<std::byte>((index * 131u + 17u) & 0xffu));
	}
	return bytes;
}

securekit::key256 fixed_key()
{
	securekit::key256 key{};
	for (std::size_t index = 0; index < key.size(); ++index)
	{
		key[index] = static_cast<std::byte>(index + 1u);
	}
	return key;
}

void write_file(const std::filesystem::path &path, std::span<const std::byte> bytes)
{
	std::ofstream out(path, std::ios::binary);
	if (!out)
	{
		throw std::runtime_error("failed to open benchmark input");
	}
	out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	if (!out)
	{
		throw std::runtime_error("failed to write benchmark input");
	}
}

securekit::bytes read_file(const std::filesystem::path &path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
	{
		throw std::runtime_error("failed to open benchmark output");
	}

	securekit::bytes bytes;
	char ch = 0;
	while (in.get(ch))
	{
		bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
	}
	if (!in.eof())
	{
		throw std::runtime_error("failed to read benchmark output");
	}
	return bytes;
}

template <typename Function>
measurement measure(std::string name, std::size_t iterations, std::size_t bytes, Function function)
{
	std::uint64_t guard = 0;
	const auto start = steady_clock_t::now();
	for (std::size_t index = 0; index < iterations; ++index)
	{
		guard += function(index) + index;
	}
	const auto stop = steady_clock_t::now();
	const std::chrono::duration<double> elapsed = stop - start;
	return {std::move(name), iterations, bytes, elapsed.count(), guard};
}

void print(const measurement &value)
{
	std::cout << value.name << " iterations=" << value.iterations << " bytes=" << value.bytes
	          << " seconds=" << value.seconds << " guard=" << value.guard << '\n';
}

} // namespace

int main()
try
{
	const securekit::bytes payload = make_bytes(64u * 1024u);
	const securekit::bytes file_payload = make_bytes(2u * 1024u * 1024u);
	const securekit::bytes aad = make_bytes(32u);
	const securekit::key256 key = fixed_key();

	std::cout << "securekit benchmark\n";

	print(measure("sha256_64k", 128u, payload.size() * 128u, [&](std::size_t) {
		const std::array<std::byte, 32u> digest = securekit::sha256(payload);
		return static_cast<std::uint64_t>(std::to_integer<unsigned char>(digest.front()));
	}));

	print(measure("aead_roundtrip_64k", 64u, payload.size() * 64u, [&](std::size_t) {
		const securekit::bytes packet = securekit::encrypt(payload, key, aad);
		const securekit::bytes opened = securekit::decrypt(packet, key, aad);
		if (opened != payload)
		{
			throw std::runtime_error("AEAD benchmark round trip mismatch");
		}
		return static_cast<std::uint64_t>(packet.size() ^ opened.size());
	}));

	const std::filesystem::path root =
	    std::filesystem::temp_directory_path() / ("securekit-bench-" + securekit::random_token(16u));
	const temp_directory temp(root);

	print(measure("file_roundtrip_2m", 3u, file_payload.size() * 3u, [&](std::size_t index) {
		const std::filesystem::path input = temp.path() / ("input-" + std::to_string(index) + ".bin");
		const std::filesystem::path sealed = temp.path() / ("sealed-" + std::to_string(index) + ".bin");
		const std::filesystem::path output = temp.path() / ("output-" + std::to_string(index) + ".bin");
		write_file(input, file_payload);
		securekit::seal_file(input, sealed, key, aad);
		securekit::open_file(sealed, output, key, aad);
		const securekit::bytes opened = read_file(output);
		if (opened != file_payload)
		{
			throw std::runtime_error("file benchmark round trip mismatch");
		}
		return static_cast<std::uint64_t>(std::filesystem::file_size(sealed) ^ opened.size());
	}));

	return 0;
}
catch (const std::exception &error)
{
	std::cerr << error.what() << '\n';
	return 1;
}
