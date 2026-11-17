#pragma once
#include "skeet/config_storage/storage.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace skeet {

struct NamedConfigRecord {
    std::string name; // UTF-8 bytes, as produced by 43336ED2
    ConfigBytes data;
    std::uint8_t key = 0x1B; // Escape is the native unassigned preset hotkey
};
struct NamedConfigListing {
    std::vector<NamedConfigRecord> records;
    bool malformed = false;
};

// The native file starts with AF. Each entry contains two tag-1 length/byte
// blocks (name, value), followed by a tag-3 byte (the preset hotkey).
NamedConfigListing parse_named_config_container(std::span<const std::byte> bytes);
ConfigBytes serialize_named_config_container(std::span<const NamedConfigRecord> records);

// Native 434798CC is an initialized UNICODE_STRING naming the single store.
// These methods correspond to NtReadFile, WriteFile, and NtDeleteFile for that
// already selected store; choosing or manufacturing a different path is outside
// this interface. Native callers ignore write/delete status.
class NamedConfigStorageBackend {
public:
    virtual ~NamedConfigStorageBackend() = default;
    virtual std::optional<ConfigBytes> read_container() = 0;
    virtual void write_container(std::span<const std::byte> bytes) = 0;
    virtual void remove_container() = 0;
};

class NamedConfigStore {
public:
    explicit NamedConfigStore(NamedConfigStorageBackend& backend) : backend_(backend) {}
    // 4339512E lists physical entries, retaining any valid prefix before an
    // error. A malformed file is removed after parsing, as in the native code.
    std::vector<NamedConfigRecord> list();
    // 433C1027 stops at the first matching name, before parsing later records.
    ConfigBytes load(std::string_view name);
    // 43397931 replaces this name, preserves bytewise name order, and assigns
    // Escape if another name already owns the requested key. Empty data deletes.
    void save(std::string_view name, std::span<const std::byte> data, std::uint8_t key);
    void erase(std::string_view name) {
        save(name, {}, 0x1B);
    }

private:
    NamedConfigStorageBackend& backend_;
};

} // namespace skeet
