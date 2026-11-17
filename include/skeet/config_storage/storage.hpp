#pragma once
#include "skeet/config.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace skeet {

using ConfigBytes = std::vector<std::byte>;
inline constexpr std::uint32_t config_stream_magic = 0xDEC00D60;
inline constexpr std::uint32_t config_stream_version = 2;
inline constexpr std::uint32_t config_stream_hash_mask = 0x92CA5DF1;

struct ConfigRecordKey {
    std::uint16_t size;
    std::uint8_t type;
    std::uint32_t hash;
};

struct ConfigRecordView {
    ConfigRecordKey key;
    std::uint8_t reserved;
    std::size_t offset;
    std::span<const std::byte> value;
};

// 43367569 accepts any version word and translates hashes with header word 2.
// Truncated records are rejected before reading beyond the supplied stream.
std::optional<ConfigRecordView> find_config_record(std::span<const std::byte> stream,
                                                   ConfigRecordKey key) noexcept;
void initialize_config_stream(ConfigBytes& stream);
// The padding byte in each newly appended native record is zero initialized.
void append_config_record(ConfigBytes& stream, ConfigRecordKey key,
                          std::span<const std::byte> value);

// Only the descriptor's serialized payload is exposed. Hotkey links and
// callbacks precede that payload and do not belong to the file representation.
std::span<std::byte> config_value_bytes(ConfigDescriptor& descriptor) noexcept;
std::span<const std::byte> config_value_bytes(const ConfigDescriptor& descriptor) noexcept;
bool config_value_supported(const ConfigDescriptor& descriptor) noexcept;
ConfigRecordKey config_record_key(const ConfigDescriptor& descriptor) noexcept;
// 43482286 constructs an empty tag-12 vector and registers metadata size 16.
void initialize_blob_config(ConfigLink& registry, ConfigBlob& descriptor, std::uint32_t hash);
// Complete 433E1D08 dispatch; tag-12 values of 65535 bytes or larger are omitted.
bool serialize_config_value(const ConfigDescriptor& descriptor, ConfigBytes& output);
bool config_value_equals(const ConfigDescriptor& descriptor,
                         std::span<const std::byte> value) noexcept;
bool deserialize_config_value(ConfigDescriptor& descriptor, std::span<const std::byte> stream,
                              bool preserve_hotkey_toggle) noexcept;
// Native return convention is heterogeneous: fixed values return changed;
// tag 12 returns whether its previous vector already equalled the found record.

using ConfigVisitor = void (*)(void*, ConfigDescriptor&);
void visit_config_registry(ConfigLink& sentinel, ConfigVisitor visitor, void* context);
ConfigDescriptor* find_config_by_value(ConfigLink& sentinel, const void* value) noexcept;

// 433D0EB1 emits every registered value without a baseline, or only values
// differing from a matching baseline record. Registry order is preserved.
// False identifies an unsupported descriptor instead of inventing its layout.
bool serialize_config_registry(ConfigLink& sentinel, ConfigBytes& stream,
                               std::span<const std::byte> baseline = {});
struct ConfigLoadResult {
    std::size_t visited = 0;
    std::size_t matched = 0;
    std::size_t changed = 0;
    std::size_t unsupported = 0;
};
ConfigLoadResult deserialize_config_registry(ConfigLink& sentinel,
                                             std::span<const std::byte> stream,
                                             bool preserve_hotkey_toggle);

// 4333CEB7/4341A2C6 use the standard alphabet. The decoder skips characters
// outside that alphabet and stops at the first '=' byte, including partial groups.
std::string encode_config_export(std::span<const std::byte> stream);
ConfigBytes decode_config_export(std::string_view encoded);
std::optional<std::string> save_config_export(ConfigLink& sentinel,
                                              std::span<const std::byte> baseline = {});
// Native load callers first restore the baseline, then apply the selected stream.
ConfigLoadResult load_config_export(ConfigLink& sentinel, std::string_view encoded,
                                    std::span<const std::byte> baseline,
                                    bool preserve_hotkey_toggle);

} // namespace skeet
