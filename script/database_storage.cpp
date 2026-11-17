#include "skeet/script/database.hpp"
#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_HEADER_FILE_ONLY
#include "third_party/miniz/miniz.c"
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace skeet::script {
namespace {
void* zallocate(void*, std::size_t count, std::size_t size) {
    return std::calloc(count, size);
}
void zrelease(void*, void* pointer) {
    std::free(pointer);
}
std::uint32_t word(std::span<const std::byte> bytes, std::size_t offset) {
    std::uint32_t value = 0;
    for (int shift = 0; shift < 32; shift += 8)
        value |= std::to_integer<std::uint32_t>(bytes[offset++]) << shift;
    return value;
}
void putword(DatabaseBytes& bytes, std::size_t offset, std::uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8)
        bytes[offset++] = std::byte(value >> shift);
}
mz_stream stream() {
    mz_stream result{};
    result.zalloc = zallocate;
    result.zfree = zrelease;
    return result;
}
}

DatabaseBytes pack_database_file(std::span<const std::byte> bytes) {
    if (bytes.empty() || bytes.size() >= 0x40000000u)
        return {};
    // 4340750F allocates source size + 0x208, allowing source size + 0x200
    // compressed bytes after its eight-byte bD envelope.
    DatabaseBytes output(bytes.size() + 0x208);
    putword(output, 0, 0x00014462);
    auto state = stream();
    state.next_in = reinterpret_cast<const unsigned char*>(bytes.data());
    state.avail_in = static_cast<unsigned>(bytes.size());
    state.next_out = reinterpret_cast<unsigned char*>(output.data() + 8);
    state.avail_out = static_cast<unsigned>(output.size() - 8);
    const auto initialized =
        mz_deflateInit2(&state, MZ_DEFAULT_COMPRESSION, MZ_DEFLATED, 15, 9, MZ_DEFAULT_STRATEGY);
    if (initialized == MZ_OK) {
        const auto result = mz_deflate(&state, MZ_FINISH);
        if (result == MZ_STREAM_END) {
            output[3] |= std::byte{1};
            putword(output, 4, static_cast<std::uint32_t>(bytes.size()));
            output.resize(state.total_out + 8);
        }
        mz_deflateEnd(&state);
    }
    return output;
}

DatabaseBytes unpack_database_file(std::span<const std::byte> bytes) {
    if (bytes.size() > 8 && bytes[0] == std::byte{0x62} && bytes[1] == std::byte{0x44}) {
        const auto size = word(bytes, 4);
        if ((bytes[3] & std::byte{1}) == std::byte{0} || !size)
            return {bytes.begin() + 8, bytes.end()};
        DatabaseBytes output(size);
        auto state = stream();
        state.next_in = reinterpret_cast<const unsigned char*>(bytes.data() + 8);
        state.avail_in = static_cast<unsigned>(bytes.size() - 8);
        state.next_out = reinterpret_cast<unsigned char*>(output.data());
        state.avail_out = size;
        const auto initialized = mz_inflateInit2(&state, 15);
        if (initialized == MZ_OK) {
            const auto result = mz_inflate(&state, MZ_FINISH);
            const bool valid = result == MZ_STREAM_END && state.total_out == size;
            mz_inflateEnd(&state);
            if (valid)
                return output;
        }
    }
    return {bytes.begin(), bytes.end()};
}
} // namespace skeet::script
