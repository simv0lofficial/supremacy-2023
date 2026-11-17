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

extern "C" {
__declspec(dllimport) ULONG NTAPI RtlGetCurrentDirectory_U(ULONG, PWSTR);
BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, PVOID);
NTSTATUS NTAPI NtQueryInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
NTSTATUS NTAPI NtSetInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
}

inline const FILE_INFORMATION_CLASS FileStandardInformation = static_cast<FILE_INFORMATION_CLASS>(5);
inline const FILE_INFORMATION_CLASS FileEndOfFileInformation = static_cast<FILE_INFORMATION_CLASS>(20);

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
