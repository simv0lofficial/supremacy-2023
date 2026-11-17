#include "skeet/script/database.hpp"
#include "detail.hpp"
#include <array>
#include <limits>
#include <optional>
#include <stdexcept>
#include <windows.h>
#include <winternl.h>

extern "C" {
NTSTATUS NTAPI NtReadFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK,
                          PVOID, ULONG, PLARGE_INTEGER, PULONG);
NTSTATUS NTAPI NtQueryInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
NTSTATUS NTAPI NtSetInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, PVOID);
}

inline const FILE_INFORMATION_CLASS FileStandardInformation = static_cast<FILE_INFORMATION_CLASS>(5);
inline const FILE_INFORMATION_CLASS FileEndOfFileInformation = static_cast<FILE_INFORMATION_CLASS>(20);

namespace skeet::script {
namespace {
constexpr int registry = -10000, globals = -10002, first_upvalue = -10003;
void* (*closure_context)(void*, int) = nullptr;
struct File {
    HANDLE handle = nullptr;
    ~File() {
        if (handle)
            NtClose(handle);
    }
};
struct Path {
    UNICODE_STRING text{};
    OBJECT_ATTRIBUTES attributes{};
    bool valid;
    explicit Path(const std::filesystem::path& path)
        : valid(RtlDosPathNameToNtPathName_U(path.c_str(), &text, nullptr, nullptr) != 0) {
        InitializeObjectAttributes(&attributes, &text, 0x40, nullptr, nullptr);
    }
    ~Path() {
        if (valid)
            RtlFreeUnicodeString(&text);
    }
};
std::optional<DatabaseBytes> read_file(const std::filesystem::path& path) {
    if (path.empty())
        return {}; // Native startup UNICODE_STRING has no buffer.
    Path native(path);
    if (!native.valid)
        return {};
    File file;
    IO_STATUS_BLOCK status{};
    if (NtOpenFile(&file.handle, 0x120089, &native.attributes, &status, 7, 0x60) < 0 ||
        !file.handle)
        return {};
    struct StandardInformation {
        LARGE_INTEGER allocation, end;
        ULONG links;
        BOOLEAN deleted, directory;
    } info{};
    if (NtQueryInformationFile(file.handle, &status, &info, sizeof(info), FileStandardInformation) <
            0 ||
        info.end.QuadPart <= 0 ||
        std::uint64_t(info.end.QuadPart) > std::numeric_limits<ULONG>::max())
        return {};
    DatabaseBytes bytes(static_cast<std::size_t>(info.end.QuadPart));
    auto result = NtReadFile(file.handle, nullptr, nullptr, nullptr, &status, bytes.data(),
                             static_cast<ULONG>(bytes.size()), nullptr, nullptr);
    if (result == 0x103) {
        if (NtWaitForSingleObject(file.handle, FALSE, nullptr) < 0)
            return {};
        result = status.Status;
    }
    if (result < 0)
        return {};
    return bytes;
}
void write_file(const std::filesystem::path& path, std::span<const std::byte> bytes) {
    if (path.empty())
        return;
    Path native(path);
    if (!native.valid)
        return;
    File file;
    IO_STATUS_BLOCK status{};
    const auto opened = NtOpenFile(&file.handle, 0x12019F, &native.attributes, &status, 7, 0x60);
    if (opened < 0 || !file.handle) {
        if (NtCreateFile(&file.handle, 0x12019F, &native.attributes, &status, nullptr, 0x80, 7, 0,
                         0x60, nullptr, 0) < 0 ||
            !file.handle)
            return;
    } else {
        LARGE_INTEGER end{};
        if (NtSetInformationFile(file.handle, &status, &end, sizeof(end),
                                 FileEndOfFileInformation) < 0)
            return;
    }
    DWORD written = 0;
    WriteFile(file.handle, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr);
}
DatabaseModule& owner(void* state) {
    return *static_cast<DatabaseModule*>(closure_context(state, first_upvalue));
}
}

void DatabaseModule::initialize_process() {
    path_ = initialize_system_database_path();
}

void DatabaseModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Database requires an initialized Lua state");
    bridge_ = &bridge;
    loaded_ = false;
    const auto& api = *bridge.api;
    auto* state = bridge.state;
    closure_context = api.touserdata;
    api.createtable(state, 0, 0);
    storage_reference_ = api.ref(state, registry);
    api.createtable(state, 0, 3);
    constexpr std::array names{"write", "read", "flush"};
    constexpr std::array<LuaApi::CFunction, 3> entries{write_entry, read_entry, flush_entry};
    for (std::size_t index = 0; index < names.size(); ++index) {
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, entries[index], 1);
        api.setfield(state, -2, names[index]);
    }
    api.setfield(state, globals, "database");
}
void DatabaseModule::ensure_loaded(void* state) {
    if (loaded_)
        return;
    const auto& api = *bridge_->api;
    if (auto bytes = read_file(path_)) {
        try {
            const auto payload = unpack_database_file(*bytes);
            api.pushlightuserdata(state, this);
            api.pushcclosure(state, decode_entry, 1);
            api.pushlightuserdata(state, const_cast<std::byte*>(payload.data()));
            api.pushinteger(state, static_cast<std::ptrdiff_t>(payload.size()));
            if (api.pcall(state, 2, 1, 0) != 0) {
                detail::report_lua_error(*bridge_, state);
                api.createtable(state, 0, 0);
            }
        } catch (const std::exception& error) {
            if (bridge_->report_error)
                bridge_->report_error(bridge_->error_context, error.what());
            api.createtable(state, 0, 0);
        }
    } else
        api.createtable(state, 0, 0);
    if (api.type(state, -1) == 5) {
        api.unref(state, registry, storage_reference_);
        storage_reference_ = api.ref(state, registry);
    }
    loaded_ = true;
}
int DatabaseModule::write(void* state) {
    const auto& api = *bridge_->api;
    if (api.gettop(state) >= 2) {
        ensure_loaded(state);
        api.rawgeti(state, registry, storage_reference_);
        api.pushvalue(state, 1);
        api.pushvalue(state, 2);
        api.settable(state, -3);
        api.settop(state, -2);
        dirty_ = true;
    }
    return 0;
}
int DatabaseModule::read(void* state) {
    const auto& api = *bridge_->api;
    ensure_loaded(state);
    if (api.gettop(state) >= 1 && api.isstring(state, 1)) {
        api.rawgeti(state, registry, storage_reference_);
        if (api.type(state, -1) != 0) {
            api.pushvalue(state, 1);
            api.gettable(state, -2);
            api.remove(state, -2);
        }
    } else
        api.pushnil(state);
    return 1;
}
int DatabaseModule::flush(void* state) {
    if (!loaded_ || !dirty_)
        return 0;
    const auto& api = *bridge_->api;
    detail::CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    detail::RtlPushFrame(&guard.active);
    auto save = [&] {
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, encode_entry, 1);
        api.rawgeti(state, registry, storage_reference_);
        if (api.pcall(state, 1, 1, 0) != 0) {
            detail::report_lua_error(*bridge_, state);
            return;
        }
        try {
            std::size_t length = 0;
            const auto* bytes = api.checklstring(state, -1, &length);
            const auto file =
                pack_database_file({reinterpret_cast<const std::byte*>(bytes), length});
            if (!file.empty())
                write_file(path_, file);
        } catch (const std::exception& error) {
            if (bridge_->report_error)
                bridge_->report_error(bridge_->error_context, error.what());
        }
    };
    detail::protected_region(save, guard);
    api.settop(state, guard.saved_top);
    detail::RtlPopFrame(&guard.active);
    dirty_ = false;
    return 0;
}
int DatabaseModule::write_entry(void* state) {
    return owner(state).write(state);
}
int DatabaseModule::read_entry(void* state) {
    return owner(state).read(state);
}
int DatabaseModule::flush_entry(void* state) {
    return owner(state).flush(state);
}
int DatabaseModule::encode_entry(void* state) {
    auto& module = owner(state);
    const auto& api = *module.bridge_->api;
    try {
        const auto bytes = encode_database_value(api, state, 1);
        api.pushlstring(state, reinterpret_cast<const char*>(bytes.data()), bytes.size());
        return 1;
    } catch (const std::exception&) {
    }
    return api.lerror(state, "%s", "33s?!");
}
int DatabaseModule::decode_entry(void* state) {
    auto& module = owner(state);
    const auto& api = *module.bridge_->api;
    const auto* bytes = static_cast<const std::byte*>(api.touserdata(state, 1));
    const auto length = static_cast<std::size_t>(api.checkinteger(state, 2));
    try {
        decode_database_value(api, state, {bytes, length});
        return 1;
    } catch (const std::exception&) {
    }
    return api.lerror(state, "%s", "33s?!");
}
} // namespace skeet::script
