#include "skeet/script/files.hpp"
#include "files_detail.hpp"
#include <new>

extern "C" __declspec(dllimport) NTSTATUS NTAPI NtReadFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID,
                                                           PIO_STATUS_BLOCK, PVOID, ULONG,
                                                           PLARGE_INTEGER, PULONG);

namespace skeet::script::files_detail {
std::unique_ptr<char[]> read_bytes(const Name& name, std::uint32_t& length) { // 0x43368E04
    NativePath path;
    if (!read_path(name, path))
        return {};
    File file;
    OBJECT_ATTRIBUTES attributes{};
    InitializeObjectAttributes(&attributes, &path.text, 0x40, nullptr, nullptr);
    IO_STATUS_BLOCK status{};
    const auto opened = NtOpenFile(&file.handle, 0x120089, &attributes, &status, 7, 0x60);
    // The native opener releases the name before querying or reading the handle.
    RtlFreeUnicodeString(&path.text);
    path.valid = false;
    if (opened < 0 || !file.handle)
        return {};
    struct StandardInformation {
        LARGE_INTEGER allocation, end;
        ULONG links;
        BOOLEAN deleted, directory;
    } information{};
    if (NtQueryInformationFile(file.handle, &status, &information, sizeof(information),
                               FileStandardInformation) < 0)
        return {};
    const auto count = information.end.LowPart;
    if (!count)
        return {};
    // 43387448 calls RtlAllocateHeap with HEAP_ZERO_MEMORY (8).
    const auto capacity = std::uint32_t(count + 1);
    if (!capacity)
        return {}; // 43387448 returns null for a zero request.
    auto bytes = std::unique_ptr<char[]>(new (std::nothrow) char[capacity]{});
    if (!bytes)
        return {};
    auto result = NtReadFile(file.handle, nullptr, nullptr, nullptr, &status, bytes.get(), count,
                             nullptr, nullptr);
    if (result == 0x103) {
        result = NtWaitForSingleObject(file.handle, FALSE, nullptr);
        if (result >= 0)
            result = status.Status;
    }
    if (result < 0 && std::uint32_t(result) != 0xC0000011u)
        return {};
    bytes[count] = 0;
    length = count;
    return bytes;
}
} // namespace skeet::script::files_detail

namespace skeet::script {
int FileModule::read(void* state) {
    if (api_->gettop(state) < 1)
        return 0;
    std::size_t length = 0;
    const auto* path = api_->checklstring(state, 1, &length);
    if (!path || !length || length >= 260)
        return 0;
    const auto name = files_detail::normalize({path, length});
    std::uint32_t count = 0;
    const auto bytes = files_detail::read_bytes(name, count);
    if (!bytes)
        return 0;
    api_->pushlstring(state, bytes.get(), count);
    return 1;
}
} // namespace skeet::script
