#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winternl.h>

namespace skeet::script::files_detail {
using Name = std::array<char, 260>;
Name normalize(std::string_view);
struct NativePath {
    UNICODE_STRING text{};
    bool valid = false;
    NativePath() = default;
    NativePath(const NativePath&) = delete;
    NativePath& operator=(const NativePath&) = delete;
    ~NativePath();
    void set(const wchar_t*);
};
struct File {
    HANDLE handle = nullptr;
    ~File();
};
bool read_path(const Name&, NativePath&);
bool write_path(const Name&, NativePath&);
HANDLE open_write(NativePath&);
std::unique_ptr<char[]> read_bytes(const Name&, std::uint32_t& length);
} // namespace skeet::script::files_detail
