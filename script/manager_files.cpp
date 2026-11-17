#include "skeet/script/manager.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winternl.h>
#include <array>
#include <limits>
#include <stdexcept>

typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

extern "C" {
__declspec(dllimport) ULONG NTAPI RtlGetCurrentDirectory_U(ULONG, PWSTR);
__declspec(dllimport) NTSTATUS NTAPI NtQueryDirectoryFile(HANDLE, HANDLE, PIO_APC_ROUTINE,
                                                         PVOID, PIO_STATUS_BLOCK, PVOID, ULONG,
                                                         FILE_INFORMATION_CLASS, BOOLEAN,
                                                         PUNICODE_STRING, BOOLEAN);
BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, PVOID);
}

namespace skeet::script {
namespace {
static_assert(offsetof(FILE_DIRECTORY_INFORMATION, LastWriteTime) == 0x18);
static_assert(offsetof(FILE_DIRECTORY_INFORMATION, FileName) == 0x40);
class NativeScriptFiles final : public ScriptFileSystem {
public:
    std::optional<std::u16string> current_directory() override {
        std::array<wchar_t, 261> directory{};
        if (!RtlGetCurrentDirectory_U(0x208, directory.data()))
            return std::nullopt;
        return std::u16string(reinterpret_cast<const char16_t*>(directory.data()));
    }
    std::vector<ScriptDirectoryEntry> enumerate_lua(std::u16string_view directory) override {
        std::vector<ScriptDirectoryEntry> result;
        std::wstring path(reinterpret_cast<const wchar_t*>(directory.data()), directory.size());
        UNICODE_STRING native_path{};
        // The native result is BOOLEAN; retain its low byte even though
        // MinGW's import declaration exposes a full BOOL return value.
        const auto converted = static_cast<BOOLEAN>(
            RtlDosPathNameToNtPathName_U(path.c_str(), &native_path, nullptr, nullptr));
        if (!converted)
            return result;
        OBJECT_ATTRIBUTES attributes{};
        InitializeObjectAttributes(&attributes, &native_path, 0x40, nullptr, nullptr);
        IO_STATUS_BLOCK status{};
        HANDLE handle = nullptr;
        const auto opened = NtOpenFile(&handle, 0x100001, &attributes, &status, 3, 0x21);
        RtlFreeUnicodeString(&native_path);
        if (opened < 0 || !handle)
            return result;
        struct Close {
            HANDLE handle;
            ~Close() {
                NtClose(handle);
            }
        } close{handle};
        wchar_t pattern_data[] = L"*.lua";
        UNICODE_STRING pattern{10, 12, pattern_data};
        std::vector<std::byte> buffer(0x400);
        bool restart = true;
        for (;;) {
            auto code = NtQueryDirectoryFile(handle, nullptr, nullptr, nullptr, &status,
                                             buffer.data(), ULONG(buffer.size()),
                                             FileDirectoryInformation, FALSE, &pattern, restart);
            if (code == 0x103) {
                code = NtWaitForSingleObject(handle, FALSE, nullptr);
                if (code >= 0)
                    code = status.Status;
            }
            if (std::uint32_t(code) == 0x80000005u || std::uint32_t(code) == 0xC0000004u) {
                if (buffer.size() > std::numeric_limits<ULONG>::max() / 2)
                    throw std::length_error("Script directory entry exceeds NT query capacity");
                buffer.resize(buffer.size() * 2);
                continue;
            }
            if (std::uint32_t(code) == 0x80000006u || code < 0)
                break;
            std::size_t offset = 0;
            for (;;) {
                if (offset + offsetof(FILE_DIRECTORY_INFORMATION, FileName) > buffer.size())
                    throw std::runtime_error("Truncated NT directory entry");
                const auto& file =
                    *reinterpret_cast<const FILE_DIRECTORY_INFORMATION*>(buffer.data() + offset);
                const auto size = std::size_t(file.FileNameLength);
                if (offset + offsetof(FILE_DIRECTORY_INFORMATION, FileName) + size >
                        buffer.size() ||
                    size % 2)
                    throw std::runtime_error("Invalid NT script filename length");
                result.push_back({{reinterpret_cast<const char16_t*>(file.FileName), size / 2},
                                  std::uint64_t(file.LastWriteTime.QuadPart)});
                if (!file.NextEntryOffset)
                    break;
                offset += file.NextEntryOffset;
            }
            restart = false;
        }
        return result;
    }
};
}
ScriptFileSystem& system_script_files() noexcept {
    static NativeScriptFiles files;
    return files;
}
} // namespace skeet::script
