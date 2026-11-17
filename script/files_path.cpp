#include "files_detail.hpp"
#include <cstring>
#include <cwchar>


namespace skeet::script::files_detail {
Name normalize(std::string_view input) {
    Name result{};
    std::memcpy(result.data(), input.data(), input.size());
    for (auto* cursor = result.data(); *cursor; ++cursor)
        if (*cursor == '/')
            *cursor = '\\';
    // The original strstr/memmove loop deliberately stops at offsets 257+.
    for (auto* pair = std::strstr(result.data(), ".."); pair && pair < result.data() + 257;
         pair = std::strstr(result.data(), ".."))
        std::memmove(pair, pair + 2, std::strlen(pair + 2) + 1);
    return result;
}
NativePath::~NativePath() {
    if (valid)
        RtlFreeUnicodeString(&text);
}
void NativePath::set(const wchar_t* path) {
    valid = RtlDosPathNameToNtPathName_U(path, &text, nullptr, nullptr) != 0;
}
File::~File() {
    if (handle)
        NtClose(handle);
}
bool read_path(const Name& name, NativePath& output) { // 0x433FBC64
    if (!name[0])
        return false;
    std::array<wchar_t, 260> converted;
    const auto count =
        MultiByteToWideChar(CP_UTF8, 0, name.data(), -1, converted.data(), int(converted.size()));
    if (!count)
        return false;
    for (int i = 0; i < count; ++i)
        if (converted[i] == L'/')
            converted[i] = L'\\';
    if (name[1] == ':')
        output.set(converted.data());
    else {
        std::array<wchar_t, 260> directory;
        const auto size = RtlGetCurrentDirectory_U(260, directory.data()) / 2;
        if (!size)
            return false;
        directory[size] = L'\\';
        if (wcscpy_s(directory.data() + size + 1, 260 - size - 1, converted.data()))
            return false;
        output.set(directory.data());
    }
    return output.valid;
}
bool write_path(const Name& name, NativePath& output) { // 0x433D0238..0x433D02C5
    std::array<wchar_t, 260> directory{};
    auto size = RtlGetCurrentDirectory_U(260, directory.data()) / 2;
    if (!size || size >= 260)
        return false;
    directory[size++] = L'\\';
    if (!MultiByteToWideChar(CP_UTF8, 0, name.data(), -1, directory.data() + size, int(260 - size)))
        return false;
    directory.back() = 0;
    output.set(directory.data());
    return output.valid;
}
HANDLE open_write(NativePath& path) { // 0x433F9C97
    if (!path.text.Buffer)
        return nullptr;
    HANDLE handle = nullptr;
    OBJECT_ATTRIBUTES attributes{};
    InitializeObjectAttributes(&attributes, &path.text, 0x40, nullptr, nullptr);
    IO_STATUS_BLOCK status{};
    if (NtOpenFile(&handle, 0x12019F, &attributes, &status, 7, 0x60) < 0 &&
        NtCreateFile(&handle, 0x12019F, &attributes, &status, nullptr, 0x80, 7, 0, 0x60, nullptr,
                     0) < 0)
        return nullptr;
    LARGE_INTEGER end{};
    NtSetInformationFile(handle, &status, &end, sizeof(end), FileEndOfFileInformation);
    return handle;
}
} // namespace skeet::script::files_detail
