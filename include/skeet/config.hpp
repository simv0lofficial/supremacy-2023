#pragma once
#include "skeet/core.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <variant>

namespace skeet {

struct ConfigLink {
    ConfigLink* next = nullptr;
    ConfigLink* previous = nullptr;
};

// Links are non-owning. The sentinel may start zeroed, as in the native image.
void append_config_link(ConfigLink& sentinel, ConfigLink& descriptor) noexcept;
// Requires a constructed circular registry, matching the original caller contract.
ConfigLink* find_config(ConfigLink& sentinel, std::uint32_t hash) noexcept;

// Native tag numbers and serialized sizes are part of the configuration format.
// Retained-image views require x86 pointers. Explicit standalone fixtures can
// use the host pointer size while preserving the same field ordering.
enum class ConfigType : std::uint8_t {
    boolean = 1,
    integer = 3,
    color = 4,
    integer_array = 6,
    hotkey = 7,
    fixed_array = 11,
    blob = 12,
};

struct ConfigMetadataInitializer {
    ConfigLink* next;
    ConfigLink* previous;
    std::uint32_t hash;
};
struct ConfigDescriptor : ConfigLink {
    std::uint32_t hash = 0;
    std::uint16_t serialized_size = 0;
    ConfigType type = ConfigType::boolean;
    std::uint8_t reserved = 0;
    ConfigDescriptor() = default;
    // Source declarations retain the original static metadata initialization;
    // this constructor is never invoked over a retained native descriptor.
    constexpr ConfigDescriptor(ConfigMetadataInitializer metadata, std::uint16_t size,
                               ConfigType tag, std::uint8_t padding) noexcept
        : ConfigLink{metadata.next, metadata.previous}, hash(metadata.hash), serialized_size(size),
          type(tag), reserved(padding) {}
};

template <class Value> struct ConfigValue : ConfigDescriptor {
    Value value{};
};

using BooleanConfig = ConfigValue<std::uint8_t>;
using IntegerConfig = ConfigValue<std::int32_t>;
using ColorConfig = ConfigValue<std::uint32_t>;
using IntegerArrayConfig = ConfigValue<std::array<std::int32_t, 9>>;
using BooleanArrayConfig = ConfigValue<std::array<std::uint8_t, 9>>;
using ScriptHashArrayConfig = ConfigValue<std::array<std::uint32_t, 128>>;
using MenuControlArrayConfig = ConfigValue<std::array<std::int32_t, 21>>;

class NativeBlobBytes;
struct SemanticBlobStorageTag {
    explicit constexpr SemanticBlobStorageTag() = default;
};
inline constexpr SemanticBlobStorageTag semantic_blob_storage{};

// A production tag12 value binds one concrete native buffer owner. The default
// route is unbound, not a vector substitute. The owned route must be selected
// explicitly by standalone source/fixture owners.
class ConfigBlobValue {
public:
    ConfigBlobValue() = default;
    explicit ConfigBlobValue(SemanticBlobStorageTag);
    ConfigBlobValue(const ConfigBlobValue&) = delete;
    ConfigBlobValue& operator=(const ConfigBlobValue&) = delete;
    ConfigBlobValue(ConfigBlobValue&&) = delete;
    ConfigBlobValue& operator=(ConfigBlobValue&&) = delete;
    void bind_native(NativeBlobBytes&);
    bool native_cells_are(const void*) const noexcept;
    void initialize_empty(); //43482286: resets native pointer triple, no old-buffer release
    std::byte* data();
    const std::byte* data() const;
    std::size_t size() const;
    bool empty() const;
    std::byte* begin() {
        return data();
    }
    std::byte* end() {
        const auto count = size();
        return count ? data() + count : data();
    }
    const std::byte* begin() const {
        return data();
    }
    const std::byte* end() const {
        const auto count = size();
        return count ? data() + count : data();
    }
    std::byte& operator[](std::size_t index) {
        return data()[index];
    }
    const std::byte& operator[](std::size_t index) const {
        return data()[index];
    }
    void assign(std::span<const std::byte>);
    template <std::contiguous_iterator Iterator> void assign(Iterator first, Iterator last) {
        if (first == last) {
            assign({});
            return;
        }
        assign(std::span<const std::byte>(std::to_address(first), std::size_t(last - first)));
    }
    void resize(std::size_t);
    void clear();
    ConfigBlobValue& operator=(const std::vector<std::byte>&);
    ConfigBlobValue& operator=(std::vector<std::byte>&&);
    operator std::span<std::byte>() {
        return {data(), size()};
    }
    operator std::span<const std::byte>() const {
        return {data(), size()};
    }

private:
    using Owned = std::vector<std::byte>;
    using Native = std::reference_wrapper<NativeBlobBytes>;
    std::variant<std::monostate, Owned, Native> storage_;
};

// This is the canonical source blob view, not the native object's physical
// payload. A production registry explicitly binds its actual16-byte header
// followed by NativeBlobCells. A registered owner must never move. Native
// buffers are released only by their actual terminal callback.
struct ConfigBlob : ConfigDescriptor {
    ConfigBlobValue value;
    ConfigBlob() = default;
    explicit ConfigBlob(SemanticBlobStorageTag route) : value(route) {}
    ~ConfigBlob();
    ConfigBlob(const ConfigBlob&) = delete;
    ConfigBlob& operator=(const ConfigBlob&) = delete;
    ConfigBlob(ConfigBlob&&) = delete;
    ConfigBlob& operator=(ConfigBlob&&) = delete;
};

struct HotkeyLink {
    HotkeyLink* next = nullptr;
    HotkeyLink* previous = nullptr;
};
struct RegisteredHotkey;
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_HOTKEY_CALLBACK __fastcall
#else
#define SKEET_HOTKEY_CALLBACK
#endif
using HotkeyCallback = void(SKEET_HOTKEY_CALLBACK*)(void*, RegisteredHotkey&);
struct RegisteredHotkey : HotkeyLink {
    void* callback_object = nullptr;
    //43405F75..43405F81: ECX=callback_object, EDX=registered node, no stack args.
    HotkeyCallback callback = nullptr;
    HotkeyBinding binding{0, HotkeyMode::always};
};

using HotkeyConfig = ConfigValue<RegisteredHotkey>;

// Native globals 0x434798BC and 0x434798C4 are independent registries.
// Descriptors must stay at a stable address after registration. Registration
// does not transfer ownership, and re-registering a linked object is invalid.
class ConfigRegistries {
public:
    ConfigRegistries(ConfigLink& descriptors, HotkeyLink& hotkeys);
    ~ConfigRegistries();
    ConfigRegistries(const ConfigRegistries&) = delete;
    ConfigRegistries& operator=(const ConfigRegistries&) = delete;
    ConfigLink& descriptors;
    HotkeyLink& hotkeys;
    // The real header remains in the native ring. Only this explicit view
    // supplies its concrete tag12 buffer operations; no header is copied.
    void bind_blob(ConfigDescriptor& native_header, ConfigBlob& canonical);
    void unbind_blob(ConfigBlob& canonical) noexcept;
    static ConfigRegistries& for_descriptors(ConfigLink& sentinel);

private:
    friend class StandaloneConfigRegistries;
    ConfigRegistries(ConfigLink&, HotkeyLink&, bool standalone);
    struct BlobBinding;
    std::vector<std::unique_ptr<BlobBinding>> blobs_;
    bool standalone_;
};

// This is an explicit standalone owner for isolated fixtures. Production
// views borrow the two independent eight-byte BSS sentinels instead.
namespace detail {
struct StandaloneConfigCells {
    ConfigLink owned_descriptors;
    HotkeyLink owned_hotkeys;
};
}
class StandaloneConfigRegistries : private detail::StandaloneConfigCells, public ConfigRegistries {
public:
    StandaloneConfigRegistries()
        : ConfigRegistries(detail::StandaloneConfigCells::owned_descriptors,
                           detail::StandaloneConfigCells::owned_hotkeys, true) {}
};

ConfigDescriptor& config_native_header(ConfigDescriptor&) noexcept;
const ConfigDescriptor& config_native_header(const ConfigDescriptor&) noexcept;
ConfigBlobValue* config_blob_value(const ConfigDescriptor&) noexcept;

static_assert(std::is_trivially_copyable_v<ConfigLink>);
static_assert(std::is_trivially_copyable_v<HotkeyLink>);
static_assert(std::is_trivially_copyable_v<ConfigDescriptor>);
static_assert(!std::is_polymorphic_v<IntegerArrayConfig>);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(ConfigLink) == 8);
static_assert(sizeof(HotkeyLink) == 8);
static_assert(sizeof(ConfigDescriptor) == 16);
static_assert(sizeof(RegisteredHotkey) == 24);
#endif

void append_hotkey(HotkeyLink& sentinel, RegisteredHotkey& binding) noexcept;
void initialize_hotkey(HotkeyLink& sentinel, RegisteredHotkey& binding, std::uint32_t key,
                       HotkeyMode mode) noexcept;

// Boolean construction registers metadata only: its existing value survives.
// All constructors preserve the descriptor's reserved byte.
void initialize_boolean_config(ConfigLink& registry, BooleanConfig& descriptor,
                               std::uint32_t hash) noexcept;
void initialize_integer_config(ConfigLink& registry, IntegerConfig& descriptor, std::uint32_t hash,
                               std::int32_t value) noexcept;
void initialize_color_config(ConfigLink& registry, ColorConfig& descriptor, std::uint32_t hash,
                             std::uint32_t value) noexcept;
void initialize_integer_array_config(ConfigLink& registry, IntegerArrayConfig& descriptor,
                                     std::uint32_t hash, std::int32_t value) noexcept;
void initialize_hotkey_config(ConfigRegistries& registries, HotkeyConfig& descriptor,
                              std::uint32_t hash, std::uint32_t key, HotkeyMode mode) noexcept;
void initialize_default_hotkey_config(ConfigRegistries& registries,
                                      HotkeyConfig& descriptor) noexcept;

} // namespace skeet
