#pragma once
#include "skeet/script/bridge.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace skeet::script {
using DatabaseBytes = std::vector<std::byte>;

// The database has its own reference-preserving Lua codec, beginning with 8E.
// Neither its value stream nor its bD file envelope is a configuration stream.
DatabaseBytes encode_database_value(const LuaApi&, void* state, int index);
void decode_database_value(const LuaApi&, void* state, std::span<const std::byte>);
DatabaseBytes pack_database_file(std::span<const std::byte>);
DatabaseBytes unpack_database_file(std::span<const std::byte>);

// 43482538 / 4349328C. Select the CPUID-keyed file under the process working
// directory and migrate the older key only when the selected file is absent.
// This is the native startup operation; it queries and may rename that file.
std::filesystem::path initialize_system_database_path();
std::uint32_t database_path_key(std::uint32_t processor_signature) noexcept;

// 43407E31 / 43336AF0 / 433206EA / 4340750F. A selected path corresponds to
// the native initialized UNICODE_STRING at 4347BD48. Construction opens nothing.
// The owner must survive until the Lua state closes; register_exports resets
// the per-state lazy-load flag and creates fresh database closures.
class DatabaseModule {
public:
    explicit DatabaseModule(std::filesystem::path path) : path_(std::move(path)) {}
    DatabaseModule(const DatabaseModule&) = delete;
    DatabaseModule& operator=(const DatabaseModule&) = delete;
    void initialize_process(); // 43482538, registered phase zero at 434792F8
    void register_exports(Bridge&);
    int write(void* state);
    int read(void* state);
    int flush(void* state);
    bool loaded() const noexcept {
        return loaded_;
    }
    bool dirty() const noexcept {
        return dirty_;
    }
    const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
    Bridge* bridge_ = nullptr;
    int storage_reference_ = -1;
    bool loaded_ = false;
    bool dirty_ = false;
    void ensure_loaded(void*);
    static int write_entry(void*);
    static int read_entry(void*);
    static int flush_entry(void*);
    static int encode_entry(void*);
    static int decode_entry(void*);
};
} // namespace skeet::script
