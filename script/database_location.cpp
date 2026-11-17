#include "skeet/script/database.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winternl.h>
#include <array>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

extern "C" {
ULONG NTAPI RtlGetCurrentDirectory_U(ULONG, PWSTR);
NTSTATUS NTAPI NtQueryAttributesFile(POBJECT_ATTRIBUTES, PFILE_BASIC_INFORMATION);
NTSTATUS NTAPI NtSetInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, PVOID);
}

inline const FILE_INFORMATION_CLASS FileRenameInformation = static_cast<FILE_INFORMATION_CLASS>(10);

namespace skeet::script {
namespace {
struct NativePath {
    UNICODE_STRING text{};
    OBJECT_ATTRIBUTES attributes{};
    bool valid;
    explicit NativePath(const std::filesystem::path& path)
        : valid(RtlDosPathNameToNtPathName_U(path.c_str(), &text, nullptr, nullptr) != 0) {
        InitializeObjectAttributes(&attributes, &text, 0x40, nullptr, nullptr);
    }
    ~NativePath() {
        if (valid)
            RtlFreeUnicodeString(&text);
    }
};
struct ProcessorInfo {
    std::uint32_t eax, ebx, ecx, edx;
};
ProcessorInfo processor_info() {
#if defined(_MSC_VER)
    int registers[4]{};
    __cpuidex(registers, 1, 0);
    return {std::uint32_t(registers[0]), std::uint32_t(registers[1]), std::uint32_t(registers[2]),
            std::uint32_t(registers[3])};
#else
    ProcessorInfo result{};
    __cpuid_count(1, 0, result.eax, result.ebx, result.ecx, result.edx);
    return result;
#endif
}
std::filesystem::path keyed_path(std::uint32_t key) { // 4349328C
    std::array<wchar_t, 260> directory{};
    // The native call passes 104h bytes although its cleared buffer is 208h.
    // It does not branch on RtlGetCurrentDirectory_U's return value.
    (void)RtlGetCurrentDirectory_U(0x104, directory.data());
    std::wstring path(directory.data());
    path += L"\\csgo\\cfg\\";
    path += std::to_wstring(key);
    path += L"7.cfg";
    return std::filesystem::path(std::move(path));
}
bool exists(NativePath& path) { // 4332CA89
    FILE_BASIC_INFORMATION basic_information{};
    return NtQueryAttributesFile(&path.attributes, &basic_information) >= 0;
}
void migrate(NativePath& previous, NativePath& selected) { // 433FA956
    const auto length = selected.text.Length;
    const auto offset = offsetof(FILE_RENAME_INFORMATION, FileName);
    DatabaseBytes storage(offset + length);
    auto& rename = *reinterpret_cast<FILE_RENAME_INFORMATION*>(storage.data());
    rename.ReplaceIfExists = FALSE;
    rename.RootDirectory = nullptr;
    rename.FileNameLength = length;
    std::memcpy(storage.data() + offset, selected.text.Buffer, length);
    HANDLE file = nullptr;
    IO_STATUS_BLOCK status{};
    if (NtOpenFile(&file, 0x110080, &previous.attributes, &status, 7, 0x4060) < 0)
        return;
    (void)NtSetInformationFile(file, &status, storage.data(), static_cast<ULONG>(storage.size()),
                               FileRenameInformation);
    NtClose(file);
}
}

std::uint32_t database_path_key(std::uint32_t signature) noexcept {
    auto hash = 0x811c9dc5u;
    for (unsigned shift = 0; shift != 32; shift += 8) {
        auto byte = (signature >> shift) & 255u;
        if (byte >= 128)
            byte |= 0xffffff00u;
        hash = (hash ^ byte) * 0x01000193u;
    }
    return hash;
}
std::filesystem::path initialize_system_database_path() { // 43482538
    const auto processor = processor_info();
    auto path = keyed_path(database_path_key(processor.eax));
    NativePath selected(path);
    if (!selected.valid)
        throw std::runtime_error("Native database path conversion failed");
    if (!exists(selected)) {
        // The native initializer queries CPUID again for the previous key.
        const auto previous_processor = processor_info();
        auto previous_path = keyed_path(
            (previous_processor.eax ^ previous_processor.ecx ^ previous_processor.edx) + 5u);
        NativePath previous(previous_path);
        if (previous.valid && exists(previous))
            migrate(previous, selected);
    }
    // The native UNICODE_STRING keeps the startup resolution even when the
    // working directory or current drive changes before a later database I/O.
    const std::wstring_view absolute(selected.text.Buffer, selected.text.Length / sizeof(wchar_t));
    if (!absolute.starts_with(L"\\??\\"))
        throw std::runtime_error("Native database path has no DOS device prefix");
    std::wstring stable = L"\\\\?\\";
    stable.append(absolute.substr(4));
    return std::filesystem::path(std::move(stable));
}
} // namespace skeet::script
