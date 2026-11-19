// Menu Component: config
#include "skeet/config.hpp"
#include "skeet/config_storage/native_blob.hpp"
#include <stdexcept>
#include <utility>
#include "skeet/config_storage/config_stream_view.hpp"
#include "skeet/config_storage/preserved.hpp"
#include "skeet/script/shutdown.hpp"
#include "skeet/config_storage/named_storage.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <cstdint>
#include "skeet/config_storage/native_blob_owner.hpp"
#include "skeet/config_storage/storage.hpp"
#include "skeet/config_storage/owner.hpp"
#include "skeet/config_storage/preset_runtime.hpp"
#include "skeet/config_storage/settings_capture.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/menu/scripts_menu.hpp"
#include "../script/files_detail.hpp"
#include <new>
#include "skeet/config_storage/settings_owner.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/menu/startup_window_name.hpp"
#include "skeet/config_storage/presets.hpp"
#include "skeet/menu/config_menu.hpp"
#include <windows.h>
#include "skeet/config_storage/settings_persistence.hpp"
#include "skeet/menu/category_config.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/script/manager.hpp"
#include "skeet/script/modules.hpp"
#include "skeet/script/ui.hpp"
#include <unordered_map>
#include <bit>
#include "skeet/engine/owner_allocation.hpp"


// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet {

// 0x433AFD92..0x433AFDA6, called by registration wrapper 0x43385AE4.
void append_config_link(ConfigLink& sentinel, ConfigLink& descriptor) noexcept {
    ConfigLink* previous = sentinel.next ? sentinel.previous : &sentinel;
    descriptor.next = &sentinel;
    descriptor.previous = previous;
    previous->next = &descriptor;
    sentinel.previous = &descriptor;
}

// Complete function 0x433E7F74..0x433E7F8D. Duplicates return the first entry.
ConfigLink* find_config(ConfigLink& sentinel, std::uint32_t hash) noexcept {
    for (auto* descriptor = sentinel.next; descriptor != &sentinel; descriptor = descriptor->next)
        if (static_cast<ConfigDescriptor*>(descriptor)->hash == hash)
            return descriptor;
    return nullptr;
}

namespace {
void initialize_descriptor(ConfigLink& registry, ConfigDescriptor& descriptor, std::uint32_t hash,
                           std::uint16_t size, ConfigType type) noexcept {
    descriptor.hash = hash;
    descriptor.serialized_size = size;
    descriptor.type = type;
    append_config_link(registry, descriptor);
}
}

// 0x4333CA03 tail-calls 0x433AFD92 with the separate hotkey sentinel.
void append_hotkey(HotkeyLink& sentinel, RegisteredHotkey& binding) noexcept {
    HotkeyLink* previous = sentinel.next ? sentinel.previous : &sentinel;
    binding.next = &sentinel;
    binding.previous = previous;
    previous->next = &binding;
    sentinel.previous = &binding;
}

// Complete 0x43486649..0x43486677. Bit 0 survives; the toggle bit is cleared.
void initialize_hotkey(HotkeyLink& sentinel, RegisteredHotkey& binding, std::uint32_t key,
                       HotkeyMode mode) noexcept {
    append_hotkey(sentinel, binding);
    binding.callback_object = nullptr;
    binding.callback = nullptr;
    binding.binding.mode = mode;
    binding.binding.packed = (key << 2) | (binding.binding.packed & 1u);
}

// Repeated inline construction, including 0x4348FAE5..0x4348FAF4.
void initialize_boolean_config(ConfigLink& registry, BooleanConfig& descriptor,
                               std::uint32_t hash) noexcept {
    initialize_descriptor(registry, descriptor, hash, 1, ConfigType::boolean);
}

// Complete 0x43482470..0x43482496.
void initialize_integer_config(ConfigLink& registry, IntegerConfig& descriptor, std::uint32_t hash,
                               std::int32_t value) noexcept {
    initialize_descriptor(registry, descriptor, hash, 4, ConfigType::integer);
    descriptor.value = value;
}

// Complete 0x434823C3..0x434823E8; channels retain their native packed order.
void initialize_color_config(ConfigLink& registry, ColorConfig& descriptor, std::uint32_t hash,
                             std::uint32_t value) noexcept {
    initialize_descriptor(registry, descriptor, hash, 4, ConfigType::color);
    descriptor.value = value;
}

// Complete 0x43480690..0x434806BD.
void initialize_integer_array_config(ConfigLink& registry, IntegerArrayConfig& descriptor,
                                     std::uint32_t hash, std::int32_t value) noexcept {
    initialize_descriptor(registry, descriptor, hash, 36, ConfigType::integer_array);
    descriptor.value.fill(value);
}

// Complete 0x434903D1..0x43490400. Only packed key/mode serialize (8 bytes);
// the registered value also owns links and a callback pair before those bytes.
void initialize_hotkey_config(ConfigRegistries& registries, HotkeyConfig& descriptor,
                              std::uint32_t hash, std::uint32_t key, HotkeyMode mode) noexcept {
    initialize_descriptor(registries.descriptors, descriptor, hash, 8, ConfigType::hotkey);
    initialize_hotkey(registries.hotkeys, descriptor.value, key, mode);
}

// Complete 0x434825FB: anonymous, unassigned Escape binding in hold mode.
void initialize_default_hotkey_config(ConfigRegistries& registries,
                                      HotkeyConfig& descriptor) noexcept {
    initialize_hotkey_config(registries, descriptor, 0, 0x1B, HotkeyMode::hold);
}

} // namespace skeet

// ============================================================================
// Subcomponent: blob_value.cpp
// ============================================================================

namespace skeet {
namespace {
[[noreturn]] void unbound_blob() {
    throw std::logic_error("Configuration blob has no explicitly selected storage owner");
}
}
ConfigBlobValue::ConfigBlobValue(SemanticBlobStorageTag) : storage_(std::in_place_type<Owned>) {}
void ConfigBlobValue::bind_native(NativeBlobBytes& bytes) {
    if (!std::holds_alternative<std::monostate>(storage_))
        throw std::logic_error("Configuration blob storage cannot be rebound or switched");
    storage_.emplace<Native>(bytes);
}
bool ConfigBlobValue::native_cells_are(const void* cells) const noexcept {
    const auto* native = std::get_if<Native>(&storage_);
    return native && &native->get().cells == cells;
}
void ConfigBlobValue::initialize_empty() {
    if (auto* native = std::get_if<Native>(&storage_)) {
        native->get().reset_empty();
        return;
    }
    if (auto* owned = std::get_if<Owned>(&storage_)) {
        owned->clear();
        return;
    }
    unbound_blob();
}
std::byte* ConfigBlobValue::data() {
    if (auto* native = std::get_if<Native>(&storage_))
        return native->get().data();
    if (auto* owned = std::get_if<Owned>(&storage_))
        return owned->data();
    unbound_blob();
}
const std::byte* ConfigBlobValue::data() const {
    if (const auto* native = std::get_if<Native>(&storage_)) {
        const auto& bytes = native->get();
        return bytes.data();
    }
    if (const auto* owned = std::get_if<Owned>(&storage_))
        return owned->data();
    unbound_blob();
}
std::size_t ConfigBlobValue::size() const {
    if (const auto* native = std::get_if<Native>(&storage_))
        return native->get().size();
    if (const auto* owned = std::get_if<Owned>(&storage_))
        return owned->size();
    unbound_blob();
}
bool ConfigBlobValue::empty() const {
    if (const auto* native = std::get_if<Native>(&storage_))
        return native->get().empty();
    if (const auto* owned = std::get_if<Owned>(&storage_))
        return owned->empty();
    unbound_blob();
}
void ConfigBlobValue::assign(std::span<const std::byte> value) {
    if (auto* native = std::get_if<Native>(&storage_)) {
        native->get().assign(value);
        return;
    }
    if (auto* owned = std::get_if<Owned>(&storage_)) {
        owned->assign(value.begin(), value.end());
        return;
    }
    unbound_blob();
}
void ConfigBlobValue::resize(std::size_t count) {
    if (auto* native = std::get_if<Native>(&storage_)) {
        native->get().resize(count);
        return;
    }
    if (auto* owned = std::get_if<Owned>(&storage_)) {
        owned->resize(count);
        return;
    }
    unbound_blob();
}
void ConfigBlobValue::clear() {
    if (auto* native = std::get_if<Native>(&storage_)) {
        native->get().clear();
        return;
    }
    if (auto* owned = std::get_if<Owned>(&storage_)) {
        owned->clear();
        return;
    }
    unbound_blob();
}
ConfigBlobValue& ConfigBlobValue::operator=(const std::vector<std::byte>& value) {
    assign(value);
    return *this;
}
ConfigBlobValue& ConfigBlobValue::operator=(std::vector<std::byte>&& value) {
    if (auto* native = std::get_if<Native>(&storage_)) {
        native->get().assign(value);
        return *this;
    }
    if (auto* owned = std::get_if<Owned>(&storage_)) {
        *owned = std::move(value);
        return *this;
    }
    unbound_blob();
}
} // namespace skeet

// ============================================================================
// Subcomponent: config_stream_view.cpp
// ============================================================================

namespace skeet {
ConfigStreamView ConfigStreamView::native_vector(const NativeConfigVector& cells) noexcept {
    return {Route::native, &cells};
}
ConfigStreamView ConfigStreamView::script_state(const script::ShutdownState& state) noexcept {
    return {Route::script, &state};
}
ConfigStreamView ConfigStreamView::snapshot(std::span<const std::byte> bytes) noexcept {
    return {Route::snapshot, nullptr, bytes};
}
ConfigStreamView ConfigStreamView::unbound() noexcept {
    return {Route::unbound, nullptr};
}
std::span<const std::byte> ConfigStreamView::bytes() const {
    switch (route_) {
    case Route::native: {
        const auto& cells = *static_cast<const NativeConfigVector*>(owner_);
        if (!cells.begin) {
            if (cells.end || cells.capacity)
                throw std::logic_error(
                    "Configuration baseline has no allocation for its published range");
            return {};
        }
        if (cells.end < cells.begin || cells.capacity < cells.end)
            throw std::logic_error("Configuration baseline has an invalid native vector range");
        return {reinterpret_cast<const std::byte*>(std::uintptr_t(cells.begin)),
                std::size_t(cells.end - cells.begin)};
    }
    case Route::script:
        return static_cast<const script::ShutdownState*>(owner_)->current_config;
    case Route::snapshot:
        return captured_;
    case Route::unbound:
        throw std::logic_error("Configuration stream owner has not been bound");
    }
    throw std::logic_error("Unknown configuration stream route");
}
bool ConfigStreamView::is_unbound() const noexcept {
    return route_ == Route::unbound;
}
bool ConfigStreamView::is_snapshot() const noexcept {
    return route_ == Route::snapshot;
}
bool ConfigStreamView::is_script_state(const script::ShutdownState& state) const noexcept {
    return route_ == Route::script && owner_ == &state;
}
bool ConfigStreamView::same_source(const ConfigStreamView& other) const noexcept {
    if (route_ != other.route_ || owner_ != other.owner_)
        return false;
    return route_ != Route::snapshot || (captured_.data() == other.captured_.data() &&
                                         captured_.size() == other.captured_.size());
}
} //namespace skeet

// ============================================================================
// Subcomponent: named_storage.cpp
// ============================================================================

namespace skeet {
namespace {
std::uint32_t named_storage_read32(std::span<const std::byte> bytes, std::size_t offset) {
    std::uint32_t result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= std::to_integer<std::uint32_t>(bytes[offset + i]) << (i * 8);
    return result;
}
void append32(ConfigBytes& bytes, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(std::byte((value >> (i * 8)) & 255));
}
bool read_block(std::span<const std::byte> bytes, std::size_t& cursor,
                std::span<const std::byte>& block) {
    if (cursor >= bytes.size() || bytes[cursor++] != std::byte{1})
        return false;
    if (bytes.size() - cursor < 4)
        return false;
    const auto size = named_storage_read32(bytes, cursor);
    cursor += 4;
    if (size > bytes.size() - cursor)
        return false;
    block = bytes.subspan(cursor, size);
    cursor += size;
    return true;
}
bool read_record(std::span<const std::byte> bytes, std::size_t& cursor, NamedConfigRecord& record) {
    std::span<const std::byte> name, data;
    if (!read_block(bytes, cursor, name) || !read_block(bytes, cursor, data))
        return false;
    if (bytes.size() - cursor < 2 || bytes[cursor] != std::byte{3})
        return false;
    record.name.assign(reinterpret_cast<const char*>(name.data()), name.size());
    record.data.assign(data.begin(), data.end());
    record.key = std::to_integer<std::uint8_t>(bytes[cursor + 1]);
    cursor += 2;
    return true;
}
void append_block(ConfigBytes& bytes, std::span<const std::byte> data) {
    if (data.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Native named configuration block exceeds uint32");
    bytes.push_back(std::byte{1});
    append32(bytes, static_cast<std::uint32_t>(data.size()));
    bytes.insert(bytes.end(), data.begin(), data.end());
}
}

NamedConfigListing parse_named_config_container(std::span<const std::byte> bytes) {
    NamedConfigListing result;
    if (bytes.empty() || bytes.front() != std::byte{0xAF}) {
        result.malformed = true;
        return result;
    }
    std::size_t cursor = 1;
    while (cursor < bytes.size()) {
        NamedConfigRecord record;
        if (!read_record(bytes, cursor, record)) {
            result.malformed = true;
            break;
        }
        result.records.push_back(std::move(record));
    }
    return result;
}

// 433E00BE iterates the native map in bytewise ascending name order. Empty
// names and empty values remain valid map nodes but are omitted from the file.
ConfigBytes serialize_named_config_container(std::span<const NamedConfigRecord> records) {
    std::map<std::string, const NamedConfigRecord*, std::less<>> ordered;
    for (const auto& record : records)
        ordered.try_emplace(record.name, &record);
    ConfigBytes output{std::byte{0xAF}};
    for (const auto& [name, record] : ordered) {
        if (name.empty() || record->data.empty())
            continue;
        append_block(output, std::as_bytes(std::span(name.data(), name.size())));
        append_block(output, record->data);
        output.push_back(std::byte{3});
        output.push_back(std::byte(record->key));
    }
    return output;
}

std::vector<NamedConfigRecord> NamedConfigStore::list() {
    const auto bytes = backend_.read_container();
    if (!bytes)
        return {};
    auto listing = parse_named_config_container(*bytes);
    if (listing.malformed)
        backend_.remove_container();
    return std::move(listing.records);
}

ConfigBytes NamedConfigStore::load(std::string_view name) {
    const auto bytes = backend_.read_container();
    if (!bytes)
        return {};
    if (bytes->empty() || bytes->front() != std::byte{0xAF}) {
        backend_.remove_container();
        return {};
    }
    std::size_t cursor = 1;
    while (cursor < bytes->size()) {
        NamedConfigRecord record;
        if (!read_record(*bytes, cursor, record)) {
            backend_.remove_container();
            return {};
        }
        if (record.name == name)
            return std::move(record.data);
    }
    return {};
}

void NamedConfigStore::save(std::string_view name, std::span<const std::byte> data,
                            std::uint8_t key) {
    name = name.substr(0, name.find('\0'));
    std::map<std::string, NamedConfigRecord, std::less<>> entries;
    for (auto& record : list())
        entries.try_emplace(record.name, std::move(record));
    if (data.empty()) {
        entries.erase(std::string(name));
    } else {
        if (key != 0x1B) {
            for (const auto& [existing_name, record] : entries) {
                const std::string_view existing(existing_name);
                if (record.key == key && existing.substr(0, existing.find('\0')) != name) {
                    key = 0x1B;
                    break;
                }
            }
        }
        entries.insert_or_assign(
            std::string(name),
            NamedConfigRecord{std::string(name), ConfigBytes(data.begin(), data.end()), key});
    }
    if (entries.empty()) {
        backend_.remove_container();
        return;
    }
    std::vector<NamedConfigRecord> ordered;
    ordered.reserve(entries.size());
    for (auto& [_, record] : entries)
        ordered.push_back(std::move(record));
    backend_.write_container(serialize_named_config_container(ordered));
}

} // namespace skeet

// ============================================================================
// Subcomponent: native_blob.cpp
// ============================================================================

namespace skeet {
namespace {
std::byte* pointer(std::uint32_t address) noexcept {
    return reinterpret_cast<std::byte*>(std::uintptr_t(address));
}
std::uint32_t address(const std::byte* value) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
}
NativeBlobBytes::NativeBlobBytes(NativeBlobCells& native, engine::JobMemory memory,
                                 std::byte& shared_empty)
    : cells(native), memory_(memory), shared_empty_(shared_empty) {
    if (sizeof(void*) != 4)
        throw std::logic_error(
            "Native configuration blobs require the original x86 pointer layout");
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Native configuration blobs require their process heap");
}
void NativeBlobBytes::reset_empty() noexcept {
    const auto empty_address = address(&shared_empty_);
    cells.limit = empty_address + 1;
    cells.first = empty_address;
    cells.last = empty_address;
}
std::byte* NativeBlobBytes::data() noexcept {
    return pointer(cells.first);
}
const std::byte* NativeBlobBytes::data() const noexcept {
    return pointer(cells.first);
}
std::byte* NativeBlobBytes::end() noexcept {
    return pointer(cells.last);
}
const std::byte* NativeBlobBytes::end() const noexcept {
    return pointer(cells.last);
}
std::size_t NativeBlobBytes::size() const noexcept {
    return std::uint32_t(cells.last - cells.first);
}
std::size_t NativeBlobBytes::capacity() const noexcept {
    return std::uint32_t(cells.limit - cells.first);
}
std::byte* NativeBlobBytes::allocate(std::size_t length) { //43387448
    auto* raw = memory_.allocate(memory_.process_heap, 8, length + 0x20);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    const auto allocation_size = static_cast<std::uint32_t>(length + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<std::byte*>(header + 16);
}
void NativeBlobBytes::free(std::byte* value) { //433837D0
    if (!value)
        return;
    std::uint32_t displacement;
    std::memcpy(&displacement, value - 8, 4);
    memory_.free(memory_.process_heap, 0, value - 16 - displacement);
}
void NativeBlobBytes::terminal_release() { //4336B1E9
    if (static_cast<std::int32_t>(cells.limit - cells.first) > 1 && cells.first)
        free(data());
}
void NativeBlobBytes::append(std::span<const std::byte> source) { //433F4815
    if (source.empty())
        return;
    const auto previous_size = size();
    const auto previous_capacity = capacity() - 1;
    const auto required = previous_size + source.size();
    if (previous_capacity < required) {
        auto next = previous_capacity > 8 ? previous_capacity * 2 : std::size_t(8);
        next = std::max(next, required);
        auto* replacement = allocate(next + 1);
        if (previous_size)
            std::memmove(replacement, data(), previous_size);
        std::memmove(replacement + previous_size, source.data(), source.size());
        replacement[required] = std::byte{};
        terminal_release();
        cells.first = address(replacement);
        cells.last = address(replacement + required);
        cells.limit = address(replacement + next + 1);
    } else {
        // The first incoming byte is copied last, matching the overlapping
        // source case in the native in-capacity append path.
        auto* last = end();
        if (source.size() > 1)
            std::memmove(last + 1, source.data() + 1, source.size() - 1);
        last[source.size()] = std::byte{};
        *last = *source.data();
        cells.last += static_cast<std::uint32_t>(source.size());
    }
}
void NativeBlobBytes::assign(std::span<const std::byte> source) { //434015F3
    const auto previous_size = size();
    if (previous_size < source.size()) {
        if (previous_size)
            std::memmove(data(), source.data(), previous_size);
        append(source.subspan(previous_size));
    } else {
        if (!source.empty())
            std::memmove(data(), source.data(), source.size());
        if (source.size() != previous_size) { //43327B3E copies the existing terminator
            std::memmove(data() + source.size(), end(), 1);
            cells.last = cells.first + static_cast<std::uint32_t>(source.size());
        }
    }
}
void NativeBlobBytes::clear() noexcept {
    if (cells.first != cells.last) {
        std::memmove(data(), end(), 1);
        cells.last = cells.first;
    }
}
void NativeBlobBytes::resize(std::size_t length) {
    const auto previous_size = size();
    if (length <= previous_size) {
        if (length != previous_size) {
            std::memmove(data() + length, end(), 1);
            cells.last = cells.first + static_cast<std::uint32_t>(length);
        }
        return;
    }
    // Resize is the adapter's byte-value operation. RAGE's recovered paths
    // use assign, whose native prefix/capacity/aliasing order is above.
    const auto available = capacity() - 1;
    if (length > available) {
        auto next = available > 8 ? available * 2 : std::size_t(8);
        next = std::max(next, length);
        auto* replacement = allocate(next + 1);
        if (previous_size)
            std::memmove(replacement, data(), previous_size);
        std::memset(replacement + previous_size, 0, length - previous_size + 1);
        terminal_release();
        cells.first = address(replacement);
        cells.last = address(replacement + length);
        cells.limit = address(replacement + next + 1);
    } else {
        std::memset(end(), 0, length - previous_size + 1);
        cells.last = cells.first + static_cast<std::uint32_t>(length);
    }
}
}

// ============================================================================
// Subcomponent: native_blob_owner.cpp
// ============================================================================

namespace skeet {
namespace {
struct NativeStringDescriptor {
    ConfigDescriptor header;
    NativeBlobCells cells;
    std::uint32_t trailing;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeStringDescriptor) == 0x20);
#endif
void release_descriptor(ConfigDescriptor& descriptor, const engine::JobMemory& memory) {
    auto* bytes = reinterpret_cast<std::byte*>(&descriptor);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement); //433837D0
}
NativeStringDescriptor* allocate_descriptor(const engine::JobMemory& memory) {
    constexpr std::uint32_t size = 0x20; //433CBD22 PUSH20h;433CBD24 POPECX
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20); //43387448
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    constexpr std::uint32_t allocated = size + 0x20;
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocated, 4);
    return reinterpret_cast<NativeStringDescriptor*>(header + 16);
}
}
NativeConfigBlobOwner::NativeConfigBlobOwner(ConfigRegistries& registry, engine::JobMemory memory,
                                             ConfigDescriptor& header, NativeBlobCells& cells,
                                             std::byte& empty)
    : registry_(registry), memory_(memory), header_(header), bytes_(cells, memory, empty) {
    configuration_.value.bind_native(bytes_);
    registry_.bind_blob(header_, configuration_);
}
std::unique_ptr<NativeConfigBlobOwner> NativeConfigBlobOwner::create(ConfigRegistries& registry,
                                                                     engine::JobMemory memory,
                                                                     std::byte& empty,
                                                                     std::uint32_t hash) {
    if (sizeof(void*) != 4)
        throw std::logic_error(
            "Dynamic native configuration strings require their original x86 layout");
    if (!memory.allocate || !memory.free || !memory.process_heap)
        throw std::invalid_argument(
            "Dynamic native configuration strings require their actual process heap");
    auto* native = allocate_descriptor(memory);
    if (!native)
        return nullptr;
    std::unique_ptr<NativeConfigBlobOwner> owner;
    try {
        owner.reset(
            new NativeConfigBlobOwner(registry, memory, native->header, native->cells, empty));
    } catch (...) {
        release_descriptor(native->header, memory);
        throw;
    }
    initialize_blob_config(registry.descriptors, owner->configuration_, hash);
    return owner;
}
NativeConfigBlobOwner::~NativeConfigBlobOwner() {
    //433ECDEE unlinks the actual header before this release. It calls only
    //433837D0 here; it does not dispatch a tag12 buffer destructor.
    registry_.unbind_blob(configuration_);
    release_descriptor(header_, memory_);
}
} // namespace skeet

// ============================================================================
// Subcomponent: owner.cpp
// ============================================================================

extern "C" __declspec(dllimport) NTSTATUS NTAPI NtReadFile(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID,
                                                           PIO_STATUS_BLOCK, PVOID, ULONG,
                                                           PLARGE_INTEGER, PULONG);
extern "C" __declspec(dllimport) NTSTATUS NTAPI NtDeleteFile(POBJECT_ATTRIBUTES);
extern "C" __declspec(dllimport) NTSTATUS NTAPI NtQueryInformationFile(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);
extern "C" __declspec(dllimport) NTSTATUS NTAPI RtlWriteRegistryValue(ULONG, PCWSTR, PCWSTR, ULONG,
                                                                      PVOID, ULONG);
using LoadedModuleCallback = void(NTAPI*)(void*, void*, BOOLEAN*);
extern "C" __declspec(dllimport) NTSTATUS NTAPI LdrEnumerateLoadedModules(ULONG,
                                                                          LoadedModuleCallback,
                                                                          void*);

namespace skeet {
namespace {
UNICODE_STRING borrowed_name(const NativeConfigPath& name) noexcept {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    // The canonical path was created as a UNICODE_STRING by the initializer;
    // its counted length is retained, including an empty allocated name.
    UNICODE_STRING out;
    out.Length = name.length;
    out.MaximumLength = name.maximum_length;
    out.Buffer = reinterpret_cast<wchar_t*>(name.buffer);
    return out;
}
struct LoaderEntry {
    LIST_ENTRY load, memory, initialization;
    void* base;
    void* entry;
    ULONG image_size;
    UNICODE_STRING full_name, base_name;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(LoaderEntry, base_name) == 0x2C);
static_assert(offsetof(NativeConfigPath, buffer) == 4);
#endif
struct ModuleVisit {
    SettingsPersistenceBackend::ModuleVisitor visitor;
    void* context;
};
void NTAPI visit_module(void* raw, void* parameter, BOOLEAN* stop) {
    if (!raw || !parameter || !stop)
        return;
    const auto& entry = *static_cast<const LoaderEntry*>(raw);
    if (!entry.base_name.Buffer || !entry.base_name.Length)
        return;
    const auto& visit = *static_cast<const ModuleVisit*>(parameter);
    if (!visit.visitor(visit.context, {reinterpret_cast<const char16_t*>(entry.base_name.Buffer),
                                       std::size_t(entry.base_name.Length) / sizeof(char16_t)}))
        *stop = TRUE;
}
}

std::optional<ConfigBytes> read_native_file(const NativeConfigPath& native_path) {
    auto path = borrowed_name(native_path);
    if (!path.Buffer)
        return std::nullopt;
    script::files_detail::File file;
    OBJECT_ATTRIBUTES attributes{};
    InitializeObjectAttributes(&attributes, &path, 0x40, nullptr, nullptr);
    IO_STATUS_BLOCK status{};
    if (NtOpenFile(&file.handle, 0x120089, &attributes, &status, 7, 0x60) < 0) {
        file.handle = nullptr;
        return std::nullopt;
    }
    struct StandardInformation {
        LARGE_INTEGER allocation, end;
        ULONG links;
        BOOLEAN deleted, directory;
    } information;
    if (NtQueryInformationFile(file.handle, &status, &information, sizeof information,
                               static_cast<FILE_INFORMATION_CLASS>(5)) < 0)
        return std::nullopt;
    const auto count = information.end.LowPart;
    // Native43387448 returns null when the wrapped allocation request is zero.
    const auto capacity = std::uint32_t(count + 1U);
    if (!capacity)
        return std::nullopt;
    std::unique_ptr<std::byte[]> memory(new (std::nothrow) std::byte[capacity]{});
    if (!memory)
        return std::nullopt;
    auto result = NtReadFile(file.handle, nullptr, nullptr, nullptr, &status, memory.get(), count,
                             nullptr, nullptr);
    if (result == 0x103) {
        result = NtWaitForSingleObject(file.handle, FALSE, nullptr);
        if (result >= 0)
            result = status.Status;
    }
    if (result < 0 && std::uint32_t(result) != 0xC0000011U)
        return std::nullopt;
    return ConfigBytes(memory.get(), memory.get() + count);
}
std::optional<ConfigBytes> NativeNamedConfigStorage::read_container() {
    return read_native_file(native_path_);
}

void NativeNamedConfigStorage::write_container(std::span<const std::byte> bytes) {
    // Reuse the actual433F9C97 open/create/truncate helper, without a second
    // UTF-8 path conversion or an owner-local alternative filename.
    script::files_detail::NativePath path;
    path.text = borrowed_name(native_path_);
    script::files_detail::File file{script::files_detail::open_write(path)};
    if (file.handle) {
        DWORD written;
        WriteFile(file.handle, bytes.data(), DWORD(bytes.size()), &written, nullptr);
    }
}

void NativeNamedConfigStorage::remove_container() {
    auto path = borrowed_name(native_path_);
    OBJECT_ATTRIBUTES attributes{};
    InitializeObjectAttributes(&attributes, &path, 0x40, nullptr, nullptr);
    (void)NtDeleteFile(&attributes);
}

void NativeSettingsPersistenceBackend::enumerate_loaded_modules(ModuleVisitor visitor,
                                                                void* context) {
    ModuleVisit visit{visitor, context};
    (void)LdrEnumerateLoadedModules(0, visit_module, &visit);
}
void NativeSettingsPersistenceBackend::write_registry_value(std::uint32_t relative_to,
                                                            std::u16string_view path,
                                                            std::u16string_view value_name,
                                                            std::uint32_t type,
                                                            std::span<const std::byte> data) {
    // SettingsPersistence supplies the native saved-window name or its owned
    // standalone string; both names and the literal Software path are terminated.
    (void)RtlWriteRegistryValue(relative_to, reinterpret_cast<const wchar_t*>(path.data()),
                                reinterpret_cast<const wchar_t*>(value_name.data()), type,
                                const_cast<std::byte*>(data.data()), ULONG(data.size()));
}

struct ConfigStorageOwner::Impl {
    std::shared_ptr<PreservedConfigStorage> native;
    ConfigRegistries& configuration;
    ui::RageClipboardApi& clipboard;
    NativeNamedConfigStorage files;
    NativeSettingsPersistenceBackend system;
    MenuSettingsCapture capture;
    SettingsPersistence persistence;
    PresetRuntime runtime;
    NamedConfigStore store;
    ConfigPresetsManager presets;
    ui::RootMenu* menu = nullptr;
    Impl(ConfigRegistries& registry, std::shared_ptr<PreservedConfigStorage> storage,
         ui::MenuWindow& window, script::Bridge& bridge, script::UiBindings& script_ui,
         ui::SettingsMenuValues settings_values, ui::ScriptsMenuValues script_values)
        : native(std::move(storage)), configuration(require_registry(registry)),
          clipboard(ui::system_rage_clipboard()), files(native->path()),
          capture(window, settings_values, script_values),
          persistence(system, *native, [this] { return capture(); }),
          runtime(window, bridge, script_ui, persistence, clipboard), store(files),
          presets(configuration, store, *native, runtime) {}
    PreservedConfigStorage& require_native() {
        if (!native)
            throw std::invalid_argument("CONFIG requires its canonical preserved native storage");
        return *native;
    }
    ConfigRegistries& require_registry(ConfigRegistries& registry) {
        if (&require_native().configuration() != &registry)
            throw std::invalid_argument(
                "CONFIG storage requires the same retained-image registry view as native startup");
        return registry;
    }
};

ConfigStorageOwner::ConfigStorageOwner(ConfigRegistries& registry,
                                       std::shared_ptr<PreservedConfigStorage> native,
                                       ui::MenuWindow& window, script::Bridge& bridge,
                                       script::UiBindings& script_ui,
                                       ui::SettingsMenuValues settings_values,
                                       ui::ScriptsMenuValues script_values)
    : impl_(std::make_unique<Impl>(registry, std::move(native), window, bridge, script_ui,
                                   settings_values, script_values)) {}
ConfigStorageOwner::~ConfigStorageOwner() = default;
ConfigRegistries& ConfigStorageOwner::configuration() noexcept {
    return impl_->configuration;
}
SettingsPersistenceState& ConfigStorageOwner::settings_state() {
    return impl_->native->settings();
}
PreservedConfigStorage& ConfigStorageOwner::native_storage() noexcept {
    return *impl_->native;
}
NamedConfigStore& ConfigStorageOwner::named() noexcept {
    return impl_->store;
}
ConfigPresetsManager& ConfigStorageOwner::presets() noexcept {
    return impl_->presets;
}
PresetRuntime& ConfigStorageOwner::runtime() noexcept {
    return impl_->runtime;
}
MenuSettingsCapture& ConfigStorageOwner::capture() noexcept {
    return impl_->capture;
}
SettingsPersistence& ConfigStorageOwner::persistence() noexcept {
    return impl_->persistence;
}
ui::RageClipboardApi& ConfigStorageOwner::clipboard() noexcept {
    return impl_->clipboard;
}
void ConfigStorageOwner::bind_menu(ui::RootMenu& menu, script::ScriptManager& scripts) {
    if (impl_->menu)
        throw std::logic_error("Configuration storage is already bound to its menu");
    impl_->runtime.bind_menu(menu, impl_->presets, scripts);
    impl_->capture.bind_menu(menu, scripts);
    impl_->menu = &menu;
}
bool ConfigStorageOwner::menu_bound_to(const ui::RootMenu& menu) const noexcept {
    return impl_->menu == &menu;
}
} // namespace skeet

// ============================================================================
// Subcomponent: preserved.cpp
// ============================================================================

namespace skeet {
namespace {
std::byte* constructed_settings(const engine::NativeDataImage& image) {
    const auto address = image.native_cell<std::uint32_t>(0x43478C68);
    if (!address)
        throw std::logic_error("CONFIG settings owner has not been initialized");
    return reinterpret_cast<std::byte*>(std::uintptr_t(address));
}
engine::UnloadSystemFunctions terminal_memory(const engine::JobMemory& memory) {
    if (!memory.free || !memory.process_heap)
        throw std::invalid_argument("Native CONFIG finalizers require their original process heap");
    engine::UnloadSystemFunctions result;
    result.free_heap = memory.free;
    result.process_heap = memory.process_heap;
    return result;
}
}
PreservedConfigStorage::PreservedConfigStorage(engine::PreservedInitialization& initialization)
    : PreservedConfigStorage(initialization.image_owner()) {}
PreservedConfigStorage::PreservedConfigStorage(std::shared_ptr<engine::NativeDataImage> image)
    : image_(image),
      configuration_(engine::require_native_data_image(image_).native_cell<ConfigLink>(0x434798BC),
                     image_->native_cell<HotkeyLink>(0x434798C4)) {
    // Early CRT callbacks can be registered before settings-owner construction.
    // This stage validates only the real BSS cells, without reading heap data.
    (void)image_->native_cell<std::uint32_t>(0x43478C68);
    (void)image_->native_cell<NativeConfigPath>(0x434798CC);
    (void)image_->native_cell<NativeConfigVector>(0x43478CB8);
}
std::uint32_t& PreservedConfigStorage::settings_cell() const {
    return image_->native_cell<std::uint32_t>(0x43478C68);
}
SettingsPersistenceState& PreservedConfigStorage::initialize_settings(engine::JobMemory memory) {
    (void)create_settings_owner(*image_, configuration_, memory);
    return settings();
}
SettingsPersistenceState& PreservedConfigStorage::settings() {
    if (!settings_) {
        auto state =
            std::make_unique<SettingsPersistenceState>(constructed_settings(*image_), 0x325);
        // Lengths follow433EEC8B's payloads; hash/type bytes remain live reads.
        constexpr std::array<std::uint16_t, 10> sizes{4, 8, 4, 4, 4, 4, 512, 4, 84, 1};
        for (std::size_t i = 0; i < sizes.size(); ++i)
            if (state->record_keys[i].size != sizes[i])
                throw std::logic_error("Native settings descriptor metadata is not constructed");
        settings_ = std::move(state);
    }
    if (reinterpret_cast<std::uintptr_t>(settings_->native_owner()) != settings_cell())
        throw std::logic_error("Constructed native settings owner was replaced or finalized");
    return *settings_;
}
NativeConfigPath& PreservedConfigStorage::path() const {
    return image_->native_cell<NativeConfigPath>(0x434798CC);
}
NativeConfigVector& PreservedConfigStorage::baseline_cells() const {
    return image_->native_cell<NativeConfigVector>(0x43478CB8);
}
std::span<const std::byte> PreservedConfigStorage::baseline() const {
    if (baseline_finalized_)
        throw std::logic_error("Native CONFIG baseline was terminally released");
    const auto& cells = baseline_cells();
    if (!cells.begin) {
        if (cells.end)
            throw std::logic_error("Native CONFIG baseline has no begin allocation");
        return {};
    }
    if (cells.end < cells.begin || cells.capacity < cells.end)
        throw std::logic_error("Native CONFIG baseline vector is not constructed");
    return {reinterpret_cast<const std::byte*>(std::uintptr_t(cells.begin)),
            std::size_t(cells.end - cells.begin)};
}
void PreservedConfigStorage::bind_source_menu_registry(ui::RootMenu& menu,
                                                       const ui::MainMenuRegistryName& name) {
    if (source_menu_ && (source_menu_ != &menu || source_registry_name_ != &name))
        throw std::logic_error("CONFIG registry name already belongs to another main menu");
    source_menu_ = &menu;
    source_registry_name_ = &name;
}
SettingsRegistryWindow PreservedConfigStorage::capture_registry_window() const {
    auto* const menu = image_->native_cell<ui::RootMenu*>(0x43476924u);
    if (!source_menu_ || !source_registry_name_ || menu != source_menu_)
        throw std::logic_error(
            "CONFIG requires the published source main menu and its registry name owner");
    return {source_registry_name_};
}
std::u16string_view
PreservedConfigStorage::registry_value_name(SettingsRegistryWindow window) const {
    if (!window.name)
        throw std::invalid_argument("Source settings registry name was not captured");
    return window.name->view();
}
NativeSettingsRegistryWindow PreservedConfigStorage::capture_native_registry_window() const {
    const auto address = image_->native_cell<std::uint32_t>(0x43476924);
    if (!address)
        throw std::logic_error("Settings persistence requires its constructed native main window");
    return {address};
}
std::u16string_view
PreservedConfigStorage::native_registry_value_name(NativeSettingsRegistryWindow window) const {
    if (!window.address)
        throw std::invalid_argument("Settings registry window was not captured");
    // RtlWriteRegistryValue receives this actual NUL-terminated inline UTF16
    // name. Reuse the saved window address even if the BSS slot was replaced
    // while433EEC8B ran; no source MenuWindow layout/address is assumed here.
    return {reinterpret_cast<const char16_t*>(std::uintptr_t(window.address) + 0xE0)};
}
void PreservedConfigStorage::finalize_settings(const engine::JobMemory& memory) {
    auto& cell = settings_cell();
    if (!cell)
        return;
    engine::free_native_allocation(reinterpret_cast<void*>(std::uintptr_t(cell)),
                                   terminal_memory(memory));
    cell = 0;
}
void PreservedConfigStorage::finalize_baseline(const engine::JobMemory& memory) {
    if (baseline_finalized_)
        throw std::logic_error("Native CONFIG baseline finalizer was invoked twice");
    const auto begin = baseline_cells().begin;
    if (begin)
        engine::free_native_allocation(reinterpret_cast<void*>(std::uintptr_t(begin)),
                                       terminal_memory(memory));
    baseline_finalized_ = true;
}
} // namespace skeet

// ============================================================================
// Subcomponent: presets.cpp
// ============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace skeet {
namespace {
std::string nul_terminated_name(std::u16string_view name) {
    auto encoded = config_name_to_utf8(name);
    encoded.resize(encoded.find('\0') == std::string::npos ? encoded.size() : encoded.find('\0'));
    return encoded;
}
std::uint32_t stored_name_hash(std::string_view name) noexcept { // 43359C70
    std::uint32_t hash = 0x811C9DC5;
    for (const unsigned char character : name) {
        if (!character)
            break;
        hash = (hash ^ character) * 0x1000193u;
    }
    return hash ^ config_stream_hash_mask;
}
}

std::uint32_t config_preset_hash(std::u16string_view name) noexcept {
    std::uint32_t hash = 0x811C9DC5;
    for (const char16_t character : name) {
        if (!character)
            break;
        hash = (hash ^ character) * 0x1000193u;
    }
    return hash ^ config_stream_hash_mask;
}
std::string config_name_to_utf8(std::u16string_view name) {
    if (name.empty())
        return {};
    if (name.size() > std::size_t(std::numeric_limits<int>::max()))
        throw std::length_error("Native preset name exceeds int");
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    const auto size =
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(name.data()),
                            static_cast<int>(name.size()), nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<std::size_t>(size), 0);
    if (size)
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(name.data()),
                            static_cast<int>(name.size()), result.data(), size, nullptr, nullptr);
    return result;
}
std::u16string config_name_from_utf8(std::string_view name) {
    if (name.empty())
        return {};
    if (name.size() > std::size_t(std::numeric_limits<int>::max()))
        throw std::length_error("Native preset name exceeds int");
    const auto size =
        MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
    std::u16string result(static_cast<std::size_t>(size), 0);
    if (size)
        MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()),
                            reinterpret_cast<wchar_t*>(result.data()), size);
    return result;
}

ConfigPresetsManager::ConfigPresetsManager(ConfigRegistries& registries, NamedConfigStore& store,
                                           std::span<const std::byte> baseline,
                                           ConfigPresetsServices& services,
                                           SettingsStateBinding settings)
    : registries_(registries), store_(store), baseline_(baseline), services_(services),
      settings_(settings) {}
ConfigPresetsManager::ConfigPresetsManager(ConfigRegistries& registries, NamedConfigStore& store,
                                           PreservedConfigStorage& preserved,
                                           ConfigPresetsServices& services)
    : registries_(registries), store_(store), preserved_(&preserved), services_(services),
      settings_(preserved) {}
std::span<const std::byte> ConfigPresetsManager::baseline_bytes() const {
    return preserved_ ? preserved_->baseline() : baseline_;
}
ConfigStreamView ConfigPresetsManager::baseline_view() const {
    return preserved_ ? ConfigStreamView::native_vector(preserved_->baseline_cells())
                      : ConfigStreamView::snapshot(baseline_);
}
void ConfigPresetsManager::unlink(RegisteredHotkey& hotkey) noexcept {
    hotkey.previous->next = hotkey.next;
    hotkey.next->previous = hotkey.previous;
}
ConfigPresetsManager::~ConfigPresetsManager() {
    for (auto& binding : bindings_)
        unlink(binding->hotkey);
}
RegisteredHotkey* ConfigPresetsManager::selected_hotkey() noexcept {
    if (menu_)
        return menu_->hotkey().value;
    return selected_index < bindings_.size() ? &bindings_[selected_index]->hotkey : nullptr;
}
void ConfigPresetsManager::refresh() {
    const auto records = store_.list();
    while (bindings_.size() > records.size()) {
        unlink(bindings_.back()->hotkey);
        bindings_.pop_back();
    }
    for (std::size_t i = 0; i < records.size(); ++i) {
        if (i < bindings_.size()) {
            auto& packed = bindings_[i]->hotkey.binding.packed;
            packed = (packed & 3u) | (std::uint32_t(records[i].key) << 2);
        } else {
            auto binding = std::make_unique<PresetBinding>();
            binding->manager = this;
            initialize_hotkey(registries_.hotkeys, binding->hotkey, records[i].key,
                              HotkeyMode::hold);
            binding->hotkey.callback_object = binding.get();
            binding->hotkey.callback = preset_hotkey;
            bindings_.push_back(std::move(binding));
        }
    }
    entries_.clear();
    entries_.reserve(records.size());
    for (std::size_t i = 0; i < records.size(); ++i) {
        // 4335C747 hashes the stored UTF-8 bytes before decoding the label;
        // the separate 433971CD update hashes the displayed UTF-16 name.
        const auto marker = static_cast<std::uint32_t>(i) |
                            (stored_name_hash(records[i].name) == active_hash ? 0x80000000u : 0u);
        entries_.push_back({config_name_from_utf8(records[i].name), marker, &bindings_[i]->hotkey});
    }
    if (menu_)
        menu_->synchronize_from_manager();
}
void ConfigPresetsManager::mark_entries() noexcept {
    for (std::size_t i = 0; i < entries_.size(); ++i)
        entries_[i].marker =
            static_cast<std::uint32_t>(i) |
            (config_preset_hash(entries_[i].name) == active_hash ? 0x80000000u : 0u);
}
void ConfigPresetsManager::update_markers() noexcept {
    mark_entries();
    if (menu_)
        menu_->update_markers();
}
void ConfigPresetsManager::select(std::uint32_t index) {
    selected_index = index;
    if (index < entries_.size() && entries_[index].name.size() <= 63)
        name = entries_[index].name;
}
void SKEET_HOTKEY_CALLBACK ConfigPresetsManager::preset_hotkey(void* context,
                                                               RegisteredHotkey& hotkey) {
    auto& binding = *static_cast<PresetBinding*>(context);
    auto& manager = *binding.manager;
    const auto found = std::find_if(manager.entries_.begin(), manager.entries_.end(),
                                    [&](const auto& entry) { return entry.hotkey == &hotkey; });
    if (found == manager.entries_.end())
        return;
    if (!manager.menu_)
        throw std::logic_error("Preset hotkey requires its native configuration menu");
    manager.menu_->load_hotkey(hotkey);
}
void ConfigPresetsManager::apply(std::span<const std::byte> stream, bool defer_loaded_event) {
    services_.before_baseline_load();
    deserialize_config_registry(registries_.descriptors, baseline_bytes(), preserve_hotkey_toggle);
    services_.after_baseline_load();
    deserialize_config_registry(registries_.descriptors, stream, preserve_hotkey_toggle);
    services_.after_values_load();
    if (!defer_loaded_event)
        services_.loaded_event();
}
bool ConfigPresetsManager::load() {
    if (name.empty() || name.size() >= 64)
        return false;
    active_hash = config_preset_hash(name);
    const auto bytes = store_.load(nul_terminated_name(name));
    if (bytes.empty())
        return false;
    apply(bytes);
    update_markers();
    services_.after_named_load();
    return true;
}
bool ConfigPresetsManager::save() {
    if (name.empty() || name.size() >= 64)
        return false;
    const auto requested_name = name; // 433B5919 captures the edit buffer before script callbacks.
    services_.flush_active_weapon_group();
    services_.saving_event(true);
    ConfigBytes bytes;
    if (!serialize_config_registry(registries_.descriptors, bytes, baseline_bytes()))
        throw std::logic_error("Unsupported registered preset descriptor");
    active_hash = config_preset_hash(requested_name);
    const auto* hotkey = selected_hotkey();
    const auto key =
        hotkey ? static_cast<std::uint8_t>(hotkey->binding.packed >> 2) : std::uint8_t(0x1B);
    store_.save(nul_terminated_name(requested_name), bytes, key);
    refresh();
    update_markers();
    services_.saving_event(false);
    services_.persist_settings();
    return true;
}
void ConfigPresetsManager::erase() {
    store_.erase(
        nul_terminated_name(name.size() < 256 ? std::u16string_view(name) : std::u16string_view{}));
    if (auto* settings = settings_.get(); settings && settings->startup_preset_hash &&
        settings->startup_preset_hash == config_preset_hash(name)) {
        settings->startup_preset_hash = 0;
        services_.persist_settings();
    }
    refresh();
}

bool ConfigPresetsManager::selected_for_startup() const {
    const auto* settings = settings_.get();
    return settings && settings->startup_preset_hash && selected_index < entries_.size() &&
           settings->startup_preset_hash == config_preset_hash(entries_[selected_index].name);
}

bool ConfigPresetsManager::set_selected_startup(bool enabled) {
    if (selected_index >= entries_.size())
        return false;
    const auto& selected = entries_[selected_index].name;
    const auto hash = config_preset_hash(selected);
    if (selected.empty() || selected.size() >= 64 || !hash ||
        std::count_if(entries_.begin(), entries_.end(), [&](const auto& entry) {
            return config_preset_hash(entry.name) == hash;
        }) != 1)
        return false;
    auto& saved = settings_.require().startup_preset_hash;
    if (enabled)
        saved = hash;
    else if (saved == hash)
        saved = 0;
    services_.persist_settings();
    return true;
}

bool ConfigPresetsManager::load_startup() {
    auto* settings = settings_.get();
    if (!settings || !settings->startup_preset_hash)
        return false;
    const auto hash = settings->startup_preset_hash;
    refresh();
    const auto matches = std::count_if(entries_.begin(), entries_.end(), [&](const auto& entry) {
        return config_preset_hash(entry.name) == hash;
    });
    if (matches != 1) {
        settings->startup_preset_hash = 0;
        services_.persist_settings();
        return false;
    }
    const auto selected = std::find_if(entries_.begin(), entries_.end(), [&](const auto& entry) {
        return config_preset_hash(entry.name) == hash;
    });
    select(static_cast<std::uint32_t>(selected - entries_.begin()));
    if (menu_) {
        menu_->list().set_selection(selected_index);
        menu_->synchronize_selection();
    }
    return load();
}
void ConfigPresetsServices::restore_defaults(ConfigLink& registry,
                                             std::span<const std::byte> baseline,
                                             const bool& preserve_hotkey_toggle) {
    before_baseline_load();
    deserialize_config_registry(registry, baseline, preserve_hotkey_toggle);
    restore_script_defaults();
    after_values_load();
    loaded_event();
}
void ConfigPresetsServices::restore_defaults(ConfigLink& registry,
                                             PreservedConfigStorage& preserved,
                                             const bool& preserve_hotkey_toggle) {
    before_baseline_load();
    deserialize_config_registry(registry, preserved.baseline(), preserve_hotkey_toggle);
    restore_script_defaults();
    after_values_load();
    loaded_event();
}
void ConfigPresetsManager::restore_default_values() {
    if (preserved_)
        services_.restore_defaults(registries_.descriptors, *preserved_, preserve_hotkey_toggle);
    else
        services_.restore_defaults(registries_.descriptors, baseline_, preserve_hotkey_toggle);
}
void ConfigPresetsManager::reset() {
    active_hash = 0;
    restore_default_values();
    update_markers();
}
bool ConfigPresetsManager::import_from_clipboard() {
    const auto bytes = decode_config_export(services_.read_clipboard_export());
    if (bytes.empty())
        return false;
    active_hash = 0;
    apply(bytes, true);
    update_markers();
    services_.loaded_event();
    return true;
}
bool ConfigPresetsManager::export_to_clipboard() {
    const auto text = save_config_export(registries_.descriptors, baseline_bytes());
    if (!text)
        throw std::logic_error("Unsupported registered preset descriptor");
    services_.write_clipboard_export(*text);
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: preset_runtime.cpp
// ============================================================================

namespace skeet {
namespace {
int no_arguments(script::Bridge&, void*) {
    return 0;
} // 43336942
}
PresetRuntime::PresetRuntime(ui::MenuWindow& window, script::Bridge& bridge,
                             script::UiBindings& script_ui, SettingsPersistence& persistence,
                             ui::RageClipboardApi& clipboard)
    : window_(window), bridge_(bridge), script_ui_(script_ui), persistence_(persistence),
      clipboard_(clipboard) {}

void PresetRuntime::bind_scripts(ConfigPresetsManager& presets, script::ScriptManager& scripts) {
    auto& state = scripts.shutdown_state();
    if (&state.bridge != &bridge_ || state.ui != &script_ui_ || &presets.services_ != this)
        throw std::invalid_argument(
            "Preset defaults require their actual script manager, bridge and UI owner");
    if ((presets_ && presets_ != &presets) ||
        (!script_defaults_.is_unbound() && !script_defaults_.is_script_state(state)))
        throw std::invalid_argument("Preset defaults are already bound to another owner");
    script_defaults_ = ConfigStreamView::script_state(state);
    presets_ = &presets;
    scripts.preserve_hotkey_toggle = &presets.preserve_hotkey_toggle;
}

void PresetRuntime::bind_menu(ui::RootMenu& menu, ConfigPresetsManager& presets,
                              script::ScriptManager& scripts) {
    auto& group = menu.rage_group_state();
    if (&group.window != &window_)
        throw std::invalid_argument("Preset runtime and RAGE must share their menu window");
    auto& modules = menu.script_modules();
    if (&modules.bridge() != &bridge_ || &modules.ui_bindings() != &script_ui_ ||
        &modules.manager() != &scripts || &menu.preset_manager() != &presets)
        throw std::invalid_argument(
            "Preset runtime and menu require the same Lua and configuration owners");
    modules.validate_config_binding(presets, *this);
    if (!group.baseline_a.same_source(presets.baseline_view()) ||
        !group.baseline_b.is_script_state(modules.shutdown_state()))
        throw std::invalid_argument(
            "Preset runtime and RAGE require the same live configuration streams");
    bind_scripts(presets, scripts);
    rage_ = &group;
    selected_group_ = &menu.selected_rage_group();
}
ui::RageWeaponGroupState& PresetRuntime::rage() const {
    if (!rage_ || !selected_group_)
        throw std::logic_error("Preset runtime requires its constructed RAGE menu");
    return *rage_;
}
const bool& PresetRuntime::preserve_hotkey_toggle() const {
    if (!presets_)
        throw std::logic_error("Preset runtime requires its preset manager");
    return presets_->preserve_hotkey_toggle;
}
void PresetRuntime::event(events::EventName name) {
    if (bridge_.state)
        script::dispatch_callbacks(bridge_, bridge_.name(name).get(), no_arguments, nullptr);
}
void PresetRuntime::before_baseline_load() { // 4340B293; name initialized at 43480995
    event(events::EventName::pre_config_load);
}
void PresetRuntime::loaded_event() { // 43349E9A; name initialized at 43480A0B
    event(events::EventName::post_config_load);
}
void PresetRuntime::saving_event(bool before) { // 433B5972 / 433B5A2B
    event(before ? events::EventName::pre_config_save : events::EventName::post_config_save);
}
void PresetRuntime::flush_active_weapon_group() { // 433EC9A4
    auto& state = rage();
    const auto selected = *selected_group_;
    if (selected >= state.groups.size())
        return;
    ConfigBytes changes;
    ui::save_category_config(window_, state.registry, "rage", "aimbot", changes,
                             state.baseline_a.bytes(), state.baseline_b.bytes());
    if (changes.size() > 12)
        state.groups[selected]->value = std::move(changes);
}
void PresetRuntime::after_baseline_load() { // 433AB1C2 -> 4337EABC
    const auto defaults = script_defaults_.bytes(); // capture this invocation's native range
    script::restore_script_ui_values(script_ui_, defaults, &preserve_hotkey_toggle());
}
void PresetRuntime::restore_script_defaults() { // 4337ED22..4337ED71
    if (!bridge_.state)
        return;
    const auto defaults = script_defaults_.bytes(); // range stays fixed during this traversal
    if (defaults.empty())
        return;
    const auto count = script_ui_.records.size();
    for (std::size_t index = 0; index < count; ++index) {
        auto& binding = script_ui_.records[index];
        if (!binding.borrowed && binding.configuration)
            deserialize_config_value(*binding.configuration, defaults, preserve_hotkey_toggle());
    }
}
void PresetRuntime::after_values_load() { // 4338A0E9
    for (auto* tab : std::span(window_.tabs))
        tab->config_loaded();
}
void PresetRuntime::after_named_load() { // 4332F0AC
    auto& state = rage();
    ui::reload_rage_weapon_groups_from_preset(state, *selected_group_);
}
void PresetRuntime::persist_settings() {
    (void)persistence_.persist(0, 0);
}

std::string PresetRuntime::read_clipboard_export() {
    auto& api = clipboard_;
    std::string result;
    if (!api.available(api.context, 1) || !api.open(api.context, 0))
        return result;
    if (const auto memory = api.get_data(api.context, 1)) {
        if (const auto text = api.wire(api.context, memory)) {
            const auto pointer = reinterpret_cast<std::uintptr_t>(text);
            const auto capacity = api.size(api.context, pointer);
            std::size_t length = 0;
            while (length < capacity && text[length])
                ++length;
            result.assign(text, length);
            api.unlock(api.context, pointer);
        }
    }
    api.close(api.context);
    return result;
}
void PresetRuntime::write_clipboard_export(std::string_view text) {
    auto& api = clipboard_;
    if (text.empty() || !api.open(api.context, 0))
        return;
    if (api.empty(api.context)) {
        if (const auto memory = api.allocate(api.context, 0, text.size() + 1)) {
            auto* target = reinterpret_cast<char*>(memory);
            std::memcpy(target, text.data(), text.size());
            target[text.size()] = '\0';
            if (!api.set_data(api.context, 1, memory))
                api.free(api.context, memory);
        }
    }
    api.close(api.context);
}
}

// ============================================================================
// Subcomponent: registry_binding.cpp
// ============================================================================

namespace skeet {
struct ConfigRegistries::BlobBinding {
    ConfigDescriptor* native;
    ConfigBlob* canonical;
};
namespace {
struct BlobIndexEntry {
    ConfigRegistries* registry;
    ConfigDescriptor* native;
    ConfigBlobValue* value;
};
// These indices contain identity pointers only. The registry owns each
// binding, while the actual native ring and buffer remain their own storage.
struct RegistryIndex {
    std::unordered_map<ConfigLink*, ConfigRegistries*> descriptors;
    std::unordered_map<HotkeyLink*, ConfigRegistries*> hotkeys;
    std::unordered_map<const ConfigDescriptor*, BlobIndexEntry> blobs;
};
RegistryIndex& index() {
    static RegistryIndex value;
    return value;
}
}
ConfigRegistries::ConfigRegistries(ConfigLink& configs, HotkeyLink& keys)
    : ConfigRegistries(configs, keys, false) {}
ConfigRegistries::ConfigRegistries(ConfigLink& configs, HotkeyLink& keys, bool standalone)
    : descriptors(configs), hotkeys(keys), standalone_(standalone) {
    auto& views = index();
    if (views.descriptors.contains(&configs) || views.hotkeys.contains(&keys))
        throw std::logic_error("Configuration sentinels already have their canonical source view");
    views.descriptors.emplace(&configs, this);
    try {
        views.hotkeys.emplace(&keys, this);
    } catch (...) {
        views.descriptors.erase(&configs);
        throw;
    }
}
ConfigRegistries::~ConfigRegistries() {
    auto& views = index();
    for (const auto& binding : blobs_) {
        views.blobs.erase(binding->native);
        views.blobs.erase(binding->canonical);
    }
    views.hotkeys.erase(&hotkeys);
    views.descriptors.erase(&descriptors);
}
ConfigRegistries& ConfigRegistries::for_descriptors(ConfigLink& sentinel) {
    const auto found = index().descriptors.find(&sentinel);
    if (found == index().descriptors.end())
        throw std::logic_error(
            "Configuration registration requires its explicit canonical registry view");
    return *found->second;
}
void ConfigRegistries::bind_blob(ConfigDescriptor& native, ConfigBlob& canonical) {
    auto& entries = index().blobs;
    const auto previous = entries.find(&native);
    if (previous != entries.end()) {
        if (previous->second.registry == this && previous->second.value == &canonical.value)
            return;
        throw std::logic_error("Native configuration descriptor already has another blob view");
    }
    if (entries.contains(&canonical))
        throw std::logic_error("Canonical blob view is already bound to another descriptor");
    if ((!standalone_ || &native != static_cast<ConfigDescriptor*>(&canonical)) &&
        !canonical.value.native_cells_are(reinterpret_cast<std::byte*>(&native) +
                                          sizeof(ConfigDescriptor)))
        throw std::invalid_argument(
            "Native blob view must reference the actual pointer triple immediately after its header");
    auto binding = std::make_unique<BlobBinding>(BlobBinding{&native, &canonical});
    blobs_.push_back(std::move(binding));
    try {
        entries.emplace(&native, BlobIndexEntry{this, &native, &canonical.value});
        if (&native != static_cast<ConfigDescriptor*>(&canonical))
            entries.emplace(&canonical, BlobIndexEntry{this, &native, &canonical.value});
    } catch (...) {
        entries.erase(&native);
        entries.erase(&canonical);
        blobs_.pop_back();
        throw;
    }
}
void ConfigRegistries::unbind_blob(ConfigBlob& canonical) noexcept {
    const auto found = std::find_if(blobs_.begin(), blobs_.end(), [&](const auto& binding) {
        return binding->canonical == &canonical;
    });
    if (found == blobs_.end())
        return;
    index().blobs.erase((*found)->native);
    index().blobs.erase(&canonical);
    blobs_.erase(found);
}
ConfigBlob::~ConfigBlob() {
    const auto found = index().blobs.find(this);
    if (found != index().blobs.end())
        found->second.registry->unbind_blob(*this);
}
ConfigDescriptor& config_native_header(ConfigDescriptor& descriptor) noexcept {
    const auto found = index().blobs.find(&descriptor);
    return found == index().blobs.end() ? descriptor : *found->second.native;
}
const ConfigDescriptor& config_native_header(const ConfigDescriptor& descriptor) noexcept {
    return config_native_header(const_cast<ConfigDescriptor&>(descriptor));
}
ConfigBlobValue* config_blob_value(const ConfigDescriptor& descriptor) noexcept {
    const auto found = index().blobs.find(&descriptor);
    return found == index().blobs.end() ? nullptr : found->second.value;
}
} // namespace skeet

// ============================================================================
// Subcomponent: settings_capture.cpp
// ============================================================================

namespace skeet {
MenuSettingsCapture::MenuSettingsCapture(const ui::MenuWindow& window,
                                         ui::SettingsMenuValues settings,
                                         ui::ScriptsMenuValues scripts) noexcept
    : window_(window), menu_color_(settings.menu_color), menu_key_(settings.menu_key),
      lock_layout_(settings.lock_layout), dpi_scale_(settings.dpi_scale),
      allow_unsafe_(scripts.allow_unsafe) {}

void MenuSettingsCapture::bind_menu(const ui::RootMenu& menu,
                                    const script::ScriptManager& scripts) {
    if (&menu.rage_group_state().window != &window_)
        throw std::invalid_argument("Settings capture and menu must share their window");
    if (&menu.script_modules().manager() != &scripts)
        throw std::invalid_argument("Settings capture requires the menu's Lua manager");
    menu_ = &menu;
    scripts_ = &scripts;
}

SettingsPersistenceInputs MenuSettingsCapture::operator()() const { //433EEC8B
    if (!menu_ || !scripts_)
        throw std::logic_error("Settings capture requires its constructed menu and script manager");
    SettingsPersistenceInputs result;
    result.global_value = std::bit_cast<std::int32_t>(menu_color_);
    result.menu_key = menu_key_.binding.packed;
    result.input_mode = lock_layout_;
    result.dpi_scale = dpi_scale_;
    const auto size = window_.size_for_persistence();
    result.menu_width = size.x;
    result.menu_height = size.y;

    std::array<std::int32_t, 21> layout{};
    std::array<std::uint32_t, 2> player_defaults{};
    ui::capture_menu_layout(menu_->layout_targets(), layout, false, player_defaults);
    std::copy_n(layout.begin() + 1, 15, result.menu_control_values.begin());
    result.menu_control_values[15] = layout[19];
    result.menu_control_values[16] = layout[20];

    const auto records = scripts_->records();
    result.capture_scripts = !records.empty();
    result.scripts.reserve(records.size());
    for (const auto& record : records)
        result.scripts.push_back({record.name, record.startup != 0});
    result.flag = allow_unsafe_;
    return result;
}
}

// ============================================================================
// Subcomponent: settings_owner.cpp
// ============================================================================

namespace skeet {
namespace {
template <class T> T& field(std::byte* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(storage + offset);
}
void append(ConfigRegistries& configuration, std::byte* storage, std::size_t offset,
            std::uint32_t hash, std::uint16_t size, ConfigType type) noexcept {
    auto& descriptor = field<ConfigDescriptor>(storage, offset);
    descriptor.hash = hash;
    descriptor.serialized_size = size;
    descriptor.type = type;
    append_config_link(configuration.descriptors, descriptor);
}
}

void construct_settings_owner(void* owner, ConfigRegistries& configuration) {
    if (!owner || reinterpret_cast<std::uintptr_t>(owner) % alignof(std::uint32_t))
        throw std::invalid_argument("CONFIG settings storage requires its native alignment");
    auto* const storage = static_cast<std::byte*>(owner);
    initialize_color_config(configuration.descriptors, field<ColorConfig>(storage, 0), 0x141CE87Fu,
                            0xFF2BCA9Fu);
    initialize_hotkey_config(configuration, field<HotkeyConfig>(storage, 0x14), 0x8BBF8EA9u, 0x2Eu,
                             HotkeyMode::hold);
    initialize_integer_config(configuration.descriptors, field<IntegerConfig>(storage, 0x3C),
                              0xD3F1456Eu, 1);
    initialize_integer_config(configuration.descriptors, field<IntegerConfig>(storage, 0x50),
                              0x27BA18FAu, 0);
    append(configuration, storage, 0x64, 0x1F495BA0u, 4, ConfigType::integer);
    append(configuration, storage, 0x78, 0xEAA92CD1u, 4, ConfigType::integer);
    append(configuration, storage, 0x8C, 0x6D868B05u, 0x200, ConfigType::integer_array);
    std::memset(storage + 0x9C, 0, 0x200);
    append(configuration, storage, 0x29C, 0xA340162Au, 4, ConfigType::integer);
    append(configuration, storage, 0x2B0, 0x580B210Eu, 0x54, ConfigType::fixed_array);
    field<std::int32_t>(storage, 0x2C0) = 9;
    append(configuration, storage, 0x314, 0xE44D893Bu, 1, ConfigType::boolean);
}

std::byte* create_settings_owner(engine::NativeDataImage& data, ConfigRegistries& configuration,
                                 engine::JobMemory memory) {
    if (&configuration.descriptors != &data.native_cell<ConfigLink>(0x434798BCu) ||
        &configuration.hotkeys != &data.native_cell<HotkeyLink>(0x434798C4u))
        throw std::invalid_argument(
            "CONFIG settings owner requires the canonical configuration registries");
    auto& active = data.native_cell<std::uint32_t>(0x43478C68u);
    if (active)
        throw std::logic_error("CONFIG settings owner is already constructed");
    if (!memory.allocate || !memory.free || !memory.process_heap)
        throw std::invalid_argument("CONFIG settings owner requires the canonical process heap");
    auto* const owner =
        static_cast<std::byte*>(engine::allocate_owner_storage(memory, settings_owner_size));
    if (!owner)
        throw std::bad_alloc{};
    construct_settings_owner(owner, configuration);
    active = std::uint32_t(reinterpret_cast<std::uintptr_t>(owner));
    return owner;
}
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(ConfigDescriptor) == 0x10);
static_assert(sizeof(HotkeyConfig) == 0x28);
static_assert(sizeof(ScriptHashArrayConfig) == 0x210);
static_assert(sizeof(MenuControlArrayConfig) == 0x64);
#endif
} //namespace skeet

// ============================================================================
// Subcomponent: settings_persistence.cpp
// ============================================================================

namespace skeet {
SettingsPersistenceState* SettingsStateBinding::get() const {
    if (!storage_)
        return standalone_;
    if (!storage_->image().native_cell<std::uint32_t>(0x43478C68u))
        return nullptr;
    return &storage_->settings();
}
SettingsPersistenceState& SettingsStateBinding::require() const {
    auto* state = get();
    if (!state)
        throw std::logic_error(
            "CONFIG settings owner has not reached its startup construction phase");
    return *state;
}
namespace {
constexpr std::array<std::size_t, 10> descriptor_offsets{0,    0x14, 0x3C,  0x50,  0x64,
                                                         0x78, 0x8C, 0x29C, 0x2B0, 0x314};
std::byte* checked_owner(std::byte* owner, std::size_t size) {
    if (!owner || size < 0x325 || reinterpret_cast<std::uintptr_t>(owner) % alignof(std::uint32_t))
        throw std::invalid_argument(
            "Constructed native settings owner has an invalid extent or alignment");
    return owner;
}
template <class T> T read(const std::byte* bytes, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, bytes + offset, sizeof value);
    return value;
}
template <class T> void write(std::byte* bytes, std::size_t offset, T value) noexcept {
    std::memcpy(bytes + offset, &value, sizeof value);
}
template <class T> T& native_value(std::byte* bytes, std::size_t size, std::size_t offset) {
    if (!bytes || offset > size || size - offset < sizeof(T))
        throw std::invalid_argument("Constructed native settings value is outside its owner");
    return *reinterpret_cast<T*>(bytes + offset);
}
template <class T> std::span<const std::byte> value_bytes(const T& value) {
    return std::as_bytes(std::span<const T>(&value, 1));
}
}

ConfigRecordKey SettingsRecordKeys::operator[](std::size_t index) const noexcept {
    if (!native_owner_)
        return owned_[index];
    const auto descriptor = native_owner_ + descriptor_offsets[index];
    return {read<std::uint16_t>(descriptor, 0xC), read<std::uint8_t>(descriptor, 0xE),
            read<std::uint32_t>(descriptor, 8)};
}
void SettingsRecordKeys::set(std::size_t index, ConfigRecordKey key) noexcept {
    if (!native_owner_) {
        owned_[index] = key;
        return;
    }
    auto* descriptor = native_owner_ + descriptor_offsets[index];
    write(descriptor, 8, key.hash);
    write(descriptor, 0xC, key.size);
    write(descriptor, 0xE, key.type);
}
SettingsPersistenceState::SettingsPersistenceState()
    : owned_(std::make_unique<SettingsPersistenceValues>()), global_value(owned_->global_value),
      menu_binding(owned_->menu_binding), input_mode(owned_->input_mode),
      dpi_scale(owned_->dpi_scale), menu_width(owned_->menu_width), menu_height(owned_->menu_height),
      script_hashes(owned_->script_hashes), optional_value(owned_->optional_value),
      menu_controls(owned_->menu_controls), flag(owned_->flag) {}
SettingsPersistenceState::SettingsPersistenceState(std::byte* owner, std::size_t size)
    : native_owner_(checked_owner(owner, size)), record_keys(native_owner_),
      global_value(native_value<std::int32_t>(native_owner_, size, 0x10)),
      menu_binding(native_value<HotkeyBinding>(native_owner_, size, 0x34)),
      input_mode(native_value<std::int32_t>(native_owner_, size, 0x4C)),
      dpi_scale(native_value<std::int32_t>(native_owner_, size, 0x60)),
      menu_width(native_value<std::int32_t>(native_owner_, size, 0x74)),
      menu_height(native_value<std::int32_t>(native_owner_, size, 0x88)),
      script_hashes(native_value<std::array<std::uint32_t, 128>>(native_owner_, size, 0x9C)),
      optional_value(native_value<std::uint32_t>(native_owner_, size, 0x2AC)),
      menu_controls(native_value<std::array<std::int32_t, 21>>(native_owner_, size, 0x2C0)),
      flag(native_value<std::uint8_t>(native_owner_, size, 0x324)) {}
namespace {
std::uint32_t script_hash(std::u16string_view name) noexcept {
    std::uint32_t hash = 0x811C9DC5;
    for (char16_t character : name) {
        if (!character)
            break;
        hash = (hash ^ character) * 0x1000193u;
    }
    return hash;
}
bool module_gate(void* context, std::u16string_view module) {
    const auto hash = settings_module_hash(module);
    if (hash != 0x0F3D9476 && hash != 0xE2280AF1)
        return true;
    *static_cast<bool*>(context) = true;
    return false;
}
}

std::uint32_t settings_module_hash(std::u16string_view module) noexcept {
    std::uint32_t hash = 0x811C9DC5;
    for (char16_t character : module) {
        if (character >= u'A' && character <= u'Z')
            character += u'a' - u'A';
        hash = (hash ^ character) * 0x1000193u;
    }
    return hash ^ config_stream_hash_mask;
}

bool serialize_settings_snapshot(SettingsPersistenceState& state,
                                 const SettingsPersistenceInputs& inputs,
                                 std::uint32_t optional_value, std::uint32_t excluded_script,
                                 ConfigBytes& output) {
    constexpr std::array<std::uint16_t, 10> sizes{4, 8, 4, 4, 4, 4, 512, 4, 84, 1};
    for (std::size_t i = 0; i < sizes.size(); ++i) {
        const auto key = state.record_keys[i];
        if (key.size != sizes[i])
            return false;
        const auto type = key.type;
        if (i == 1 ? type != 7 : (type != 1 && type != 3 && type != 4 && type != 6 && type != 11))
            return false;
    }
    state.global_value = inputs.global_value;
    state.menu_binding.packed =
        inputs.menu_key ^ ((state.menu_binding.packed ^ inputs.menu_key) & 3u);
    state.input_mode = inputs.input_mode;
    state.dpi_scale = inputs.dpi_scale;
    state.menu_width = inputs.menu_width;
    state.menu_height = inputs.menu_height;
    state.menu_controls[0] = 9;
    for (std::size_t i = 0; i < 15; ++i)
        state.menu_controls[i + 1] = inputs.menu_control_values[i];
    state.menu_controls[19] = inputs.menu_control_values[15];
    state.menu_controls[20] = inputs.menu_control_values[16];
    if (inputs.capture_scripts) {
        state.script_hashes.fill(0);
        std::size_t count = 0;
        for (const auto& script : inputs.scripts) {
            const bool starred = !script.name.empty() && script.name.front() == u'*';
            if (script.selected == starred)
                continue;
            const auto hash = script_hash(script.name);
            if (hash == excluded_script)
                continue;
            state.script_hashes[count++] = hash;
            if (count == state.script_hashes.size())
                break;
        }
    }
    state.flag = inputs.flag;
    initialize_config_stream(output);
    append_config_record(output, state.record_keys[0], value_bytes(state.global_value));
    append_config_record(output, state.record_keys[1], value_bytes(state.menu_binding));
    append_config_record(output, state.record_keys[2], value_bytes(state.input_mode));
    append_config_record(output, state.record_keys[3], value_bytes(state.dpi_scale));
    append_config_record(output, state.record_keys[4], value_bytes(state.menu_width));
    append_config_record(output, state.record_keys[5], value_bytes(state.menu_height));
    append_config_record(output, state.record_keys[6], value_bytes(state.script_hashes));
    if (optional_value) {
        state.optional_value = optional_value;
        append_config_record(output, state.record_keys[7], value_bytes(state.optional_value));
    }
    if (state.startup_preset_hash)
        append_config_record(output, startup_preset_record, value_bytes(state.startup_preset_hash));
    append_config_record(output, state.record_keys[8], value_bytes(state.menu_controls));
    append_config_record(output, state.record_keys[9], value_bytes(state.flag));
    return true;
}

SettingsPersistence::SettingsPersistence(SettingsPersistenceBackend& backend,
                                         SettingsPersistenceState& state,
                                         std::u16string_view value_name, Capture capture)
    : backend_(backend), state_(state), value_name_(value_name), capture_(std::move(capture)) {}
SettingsPersistence::SettingsPersistence(SettingsPersistenceBackend& backend,
                                         PreservedConfigStorage& preserved, Capture capture)
    : backend_(backend), state_(preserved), preserved_(&preserved), capture_(std::move(capture)) {}

bool SettingsPersistence::persist(std::uint32_t optional_value, std::uint32_t excluded_script) {
    bool blocked = false;
    backend_.enumerate_loaded_modules(module_gate, &blocked);
    if (blocked)
        return false;
    ConfigBytes output;
    output.reserve(0x1000);
    const auto window =
        preserved_ ? preserved_->capture_registry_window() : SettingsRegistryWindow{};
    auto& state = state_.require();
    if (!serialize_settings_snapshot(state, capture_(), optional_value, excluded_script, output))
        return false;
    const auto value_name =
        preserved_ ? preserved_->registry_value_name(window) : std::u16string_view(value_name_);
    backend_.write_registry_value(5, u"Software", value_name, 3, output);
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: storage.cpp
// ============================================================================

namespace skeet {
namespace {
std::uint16_t read16(std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return std::to_integer<std::uint16_t>(bytes[offset]) |
           (std::to_integer<std::uint16_t>(bytes[offset + 1]) << 8);
}
std::uint32_t read32(std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return read16(bytes, offset) | (std::uint32_t(read16(bytes, offset + 2)) << 16);
}
void write16(ConfigBytes& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = std::byte(value & 255);
    bytes[offset + 1] = std::byte(value >> 8);
}
void write32(ConfigBytes& bytes, std::size_t offset, std::uint32_t value) {
    write16(bytes, offset, std::uint16_t(value));
    write16(bytes, offset + 2, std::uint16_t(value >> 16));
}
template <class T> std::span<std::byte> bytes_of(T& value) noexcept {
    return std::as_writable_bytes(std::span<T>(&value, 1));
}
bool supported(const ConfigDescriptor& descriptor) noexcept {
    return config_value_supported(descriptor);
}
constexpr std::string_view alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

// Complete native lookup 43367569, with a bounded read at each record.
std::optional<ConfigRecordView> find_config_record(std::span<const std::byte> stream,
                                                   ConfigRecordKey key) noexcept {
    if (stream.size() <= 12 || read32(stream, 0) != config_stream_magic)
        return {};
    const auto hash = read32(stream, 8) ^ key.hash ^ config_stream_hash_mask;
    std::size_t offset = 12;
    while (offset < stream.size()) {
        if (stream.size() - offset < 8)
            return {};
        const auto size = read16(stream, offset);
        const auto type = std::to_integer<std::uint8_t>(stream[offset + 2]);
        if (stream.size() - offset - 8 < size)
            return {};
        if (read32(stream, offset + 4) == hash && type == key.type &&
            (type == 12 || size == key.size)) {
            return ConfigRecordView{{size, type, read32(stream, offset + 4)},
                                    std::to_integer<std::uint8_t>(stream[offset + 3]),
                                    offset,
                                    stream.subspan(offset + 8, size)};
        }
        offset += std::size_t(size) + 8;
        if (offset >= stream.size())
            break;
        if (stream.size() - offset < 3 || stream[offset + 2] == std::byte{8})
            break;
    }
    return {};
}

bool valid_config_stream(std::span<const std::byte> stream) noexcept {
    if (stream.size() < 12 || read32(stream, 0) != config_stream_magic)
        return false;
    auto remaining = stream.subspan(12);
    while (!remaining.empty()) {
        if (remaining.size() < 8)
            return false;
        const auto size = read16(remaining, 0);
        if (remaining.size() - 8 < size)
            return false;
        remaining = remaining.subspan(8 + size);
    }
    return true;
}

void initialize_config_stream(ConfigBytes& stream) {
    stream.resize(12);
    write32(stream, 0, config_stream_magic);
    write32(stream, 4, config_stream_version);
    write32(stream, 8, config_stream_hash_mask);
}

// 43417939; the resize preserves the prefix and initializes the new padding byte.
void append_config_record(ConfigBytes& stream, ConfigRecordKey key,
                          std::span<const std::byte> value) {
    if (value.size() != key.size)
        throw std::invalid_argument("Native configuration record size mismatch");
    const auto offset = stream.size();
    stream.resize(offset + 8 + value.size());
    write16(stream, offset, key.size);
    stream[offset + 2] = std::byte(key.type);
    write32(stream, offset + 4, key.hash);
    if (!value.empty())
        std::memcpy(stream.data() + offset + 8, value.data(), value.size());
}

std::span<std::byte> config_value_bytes(ConfigDescriptor& descriptor) noexcept {
    auto& header = config_native_header(descriptor);
    std::span<std::byte> bytes;
    switch (header.type) {
    case ConfigType::boolean:
        bytes = bytes_of(static_cast<BooleanConfig&>(header).value);
        break;
    case ConfigType::integer:
        bytes = bytes_of(static_cast<IntegerConfig&>(header).value);
        break;
    case ConfigType::color:
        bytes = bytes_of(static_cast<ColorConfig&>(header).value);
        break;
    case ConfigType::integer_array:
    case ConfigType::fixed_array:
        //433E1D08 copies exactly metadata+C bytes from native descriptor+10.
        // All ConfigValue<std::array<T,N>> retain that same plain header.
        bytes = {reinterpret_cast<std::byte*>(&header) + sizeof(ConfigDescriptor),
                 header.serialized_size};
        break;
    case ConfigType::hotkey:
        bytes = bytes_of(static_cast<HotkeyConfig&>(header).value.binding);
        break;
    case ConfigType::blob: {
        auto* value = config_blob_value(descriptor);
        return value ? std::span<std::byte>(*value) : std::span<std::byte>{};
    }
    default:
        return {};
    }
    return bytes.size() == header.serialized_size ? bytes : std::span<std::byte>{};
}
std::span<const std::byte> config_value_bytes(const ConfigDescriptor& descriptor) noexcept {
    return config_value_bytes(const_cast<ConfigDescriptor&>(descriptor));
}
bool config_value_supported(const ConfigDescriptor& descriptor) noexcept {
    return config_native_header(descriptor).type == ConfigType::blob
               ? config_blob_value(descriptor) != nullptr
               : !config_value_bytes(descriptor).empty();
}
ConfigRecordKey config_record_key(const ConfigDescriptor& descriptor) noexcept {
    const auto& header = config_native_header(descriptor);
    return {header.serialized_size, static_cast<std::uint8_t>(header.type), header.hash};
}
void initialize_blob_config(ConfigLink& registry, ConfigBlob& descriptor, std::uint32_t hash) {
    auto& owner = ConfigRegistries::for_descriptors(registry);
    if (!config_blob_value(descriptor))
        owner.bind_blob(descriptor, descriptor);
    auto& header = config_native_header(descriptor);
    header.hash = hash;
    header.serialized_size = 16;
    header.type = ConfigType::blob;
    append_config_link(registry, header);
    descriptor.value.initialize_empty();
}
bool serialize_config_value(const ConfigDescriptor& descriptor, ConfigBytes& output) {
    if (!supported(descriptor))
        return false;
    auto key = config_record_key(descriptor);
    const auto bytes = config_value_bytes(descriptor);
    if (config_native_header(descriptor).type == ConfigType::blob) {
        if (bytes.size() >= 0xFFFF)
            return true;
        key.size = static_cast<std::uint16_t>(bytes.size());
    }
    append_config_record(output, key, bytes);
    return true;
}

// 433E113E compares the entire serialized value, including both hotkey words.
bool config_value_equals(const ConfigDescriptor& descriptor,
                         std::span<const std::byte> value) noexcept {
    const auto bytes = config_value_bytes(descriptor);
    return supported(descriptor) && bytes.size() == value.size() &&
           (bytes.empty() || std::memcmp(bytes.data(), value.data(), bytes.size()) == 0);
}

// 433CFFB4 and tag-7 specialization 433FF6E4.
bool deserialize_config_value(ConfigDescriptor& descriptor, std::span<const std::byte> stream,
                              bool preserve_hotkey_toggle) noexcept {
    if (!supported(descriptor))
        return false;
    const auto record = find_config_record(stream, config_record_key(descriptor));
    if (!record)
        return false;
    const bool changed = !config_value_equals(descriptor, record->value);
    auto& header = config_native_header(descriptor);
    if (header.type == ConfigType::blob) {
        config_blob_value(descriptor)->assign(record->value);
        return !changed;
    }
    auto bytes = config_value_bytes(descriptor);
    std::uint32_t old_packed = 0;
    if (header.type == ConfigType::hotkey)
        old_packed = static_cast<HotkeyConfig&>(header).value.binding.packed;
    std::memcpy(bytes.data(), record->value.data(), bytes.size());
    if (header.type == ConfigType::hotkey && preserve_hotkey_toggle) {
        auto& packed = static_cast<HotkeyConfig&>(header).value.binding.packed;
        packed ^= (packed ^ old_packed) & 2u;
    }
    return changed;
}

void visit_config_registry(ConfigLink& sentinel, ConfigVisitor visitor, void* context) {
    for (auto* link = sentinel.next; link && link != &sentinel; link = link->next)
        visitor(context, static_cast<ConfigDescriptor&>(*link));
}
ConfigDescriptor* find_config_by_value(ConfigLink& sentinel, const void* value) noexcept {
    for (auto* link = sentinel.next; link && link != &sentinel; link = link->next) {
        auto& descriptor = static_cast<ConfigDescriptor&>(*link);
        if (descriptor.type == ConfigType::blob) {
            if (!value)
                return &descriptor;
            continue;
        }
        const auto bytes = config_value_bytes(descriptor);
        if (!bytes.empty() && bytes.data() == value)
            return &descriptor;
    }
    return nullptr;
}

// Complete registry writer 433D0EB1 and its equality/record dispatch chain.
bool serialize_config_registry(ConfigLink& sentinel, ConfigBytes& stream,
                               std::span<const std::byte> baseline) {
    for (auto* link = sentinel.next; link && link != &sentinel; link = link->next)
        if (!supported(static_cast<const ConfigDescriptor&>(*link)))
            return false;
    initialize_config_stream(stream);
    for (auto* link = sentinel.next; link && link != &sentinel; link = link->next) {
        const auto& descriptor = static_cast<const ConfigDescriptor&>(*link);
        const auto key = config_record_key(descriptor);
        const auto prior = find_config_record(baseline, key);
        if (!prior || !config_value_equals(descriptor, prior->value))
            serialize_config_value(descriptor, stream);
    }
    return true;
}
ConfigLoadResult deserialize_config_registry(ConfigLink& sentinel,
                                             std::span<const std::byte> stream,
                                             bool preserve_hotkey_toggle) {
    ConfigLoadResult result;
    for (auto* link = sentinel.next; link && link != &sentinel; link = link->next) {
        auto& descriptor = static_cast<ConfigDescriptor&>(*link);
        ++result.visited;
        if (!supported(descriptor)) {
            ++result.unsupported;
            continue;
        }
        const auto record = find_config_record(stream, config_record_key(descriptor));
        if (record) {
            ++result.matched;
            if (!config_value_equals(descriptor, record->value))
                ++result.changed;
        }
        deserialize_config_value(descriptor, stream, preserve_hotkey_toggle);
    }
    return result;
}

std::string encode_config_export(std::span<const std::byte> stream) {
    std::string output;
    output.reserve(((stream.size() + 2) / 3) * 4);
    for (std::size_t i = 0; i < stream.size(); i += 3) {
        const auto first = std::to_integer<unsigned>(stream[i]);
        const auto second = i + 1 < stream.size() ? std::to_integer<unsigned>(stream[i + 1]) : 0u;
        const auto third = i + 2 < stream.size() ? std::to_integer<unsigned>(stream[i + 2]) : 0u;
        output += alphabet[first >> 2];
        output += alphabet[((first & 3) << 4) | (second >> 4)];
        output += i + 1 < stream.size() ? alphabet[((second & 15) << 2) | (third >> 6)] : '=';
        output += i + 2 < stream.size() ? alphabet[third & 63] : '=';
    }
    return output;
}
ConfigBytes decode_config_export(std::string_view encoded) {
    ConfigBytes output;
    output.reserve(encoded.size() / 4 * 3);
    std::uint32_t accumulator = 0;
    unsigned bits = 0;
    for (const unsigned char character : encoded) {
        if (character == '=')
            break;
        const auto position = alphabet.find(char(character));
        if (position == std::string_view::npos)
            continue;
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(position);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output.push_back(std::byte((accumulator >> bits) & 255));
        }
    }
    return output;
}
std::optional<std::string> save_config_export(ConfigLink& sentinel,
                                              std::span<const std::byte> baseline) {
    ConfigBytes stream;
    if (!serialize_config_registry(sentinel, stream, baseline))
        return {};
    return encode_config_export(stream);
}
ConfigLoadResult load_config_export(ConfigLink& sentinel, std::string_view encoded,
                                    std::span<const std::byte> baseline,
                                    bool preserve_hotkey_toggle) {
    auto result = deserialize_config_registry(sentinel, baseline, preserve_hotkey_toggle);
    const auto stream = decode_config_export(encoded);
    const auto selected = deserialize_config_registry(sentinel, stream, preserve_hotkey_toggle);
    result.visited += selected.visited;
    result.matched += selected.matched;
    result.changed += selected.changed;
    result.unsupported += selected.unsupported;
    return result;
}

} // namespace skeet
