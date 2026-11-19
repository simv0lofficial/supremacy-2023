#pragma once
#include "skeet/config_storage/storage.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace skeet {
class PreservedConfigStorage;
inline constexpr ConfigRecordKey startup_preset_record{4, 3, 0xB8542862u};

struct SettingsScriptState {
    std::u16string name;
    bool selected = false;
};
struct SettingsPersistenceInputs {
    std::int32_t global_value = 0; // 43468FB0
    std::uint32_t menu_key = 0; // 43478DC8, including the native low bits
    std::uint8_t input_mode = 0; // 43467E4B
    std::int32_t dpi_scale = 0; // 43475A94
    std::int32_t menu_width = 0, menu_height = 0;
    // 433ACDC7 visits 15 controls, then the final two controls. Native slots
    // 16..18 in the resulting 21-word record retain their previous contents.
    std::array<std::int32_t, 17> menu_control_values{};
    std::vector<SettingsScriptState> scripts;
    std::uint8_t flag = 0; // 43475798
    bool capture_scripts = true;
};

struct SettingsPersistenceValues {
    std::int32_t global_value = 0;
    HotkeyBinding menu_binding{};
    std::int32_t input_mode = 0;
    std::int32_t dpi_scale = 0;
    std::int32_t menu_width = 0, menu_height = 0;
    std::array<std::uint32_t, 128> script_hashes{};
    std::uint32_t optional_value = 0;
    std::array<std::int32_t, 21> menu_controls{};
    std::uint8_t flag = 0;
};

// The ten native descriptors are not laid out like ConfigRecordKey. Reading
// through this view retains their actual hash+8/size+C/type+E fields; set()
// writes those same fields without changing links or the reserved byte.
class SettingsRecordKeys {
public:
    SettingsRecordKeys() = default;
    explicit SettingsRecordKeys(std::byte* native_owner) noexcept : native_owner_(native_owner) {}
    ConfigRecordKey operator[](std::size_t) const noexcept;
    void set(std::size_t, ConfigRecordKey) noexcept;
    static constexpr std::size_t size() noexcept {
        return 10;
    }

private:
    std::byte* native_owner_ = nullptr;
    std::array<ConfigRecordKey, 10> owned_{};
};

class SettingsPersistenceState {
    // Standalone fixtures can own values. Production construction leaves this
    // disengaged and binds every value reference directly to the native owner.
    std::unique_ptr<SettingsPersistenceValues> owned_;
    std::byte* native_owner_ = nullptr;

public:
    // Source-only preference, deliberately outside the per-preset registry.
    std::uint32_t startup_preset_hash = 0;
    SettingsPersistenceState();
    SettingsPersistenceState(std::byte* constructed_owner, std::size_t owner_size);
    SettingsPersistenceState(const SettingsPersistenceState&) = delete;
    SettingsPersistenceState& operator=(const SettingsPersistenceState&) = delete;
    SettingsRecordKeys record_keys;
    std::int32_t& global_value;
    HotkeyBinding& menu_binding;
    std::int32_t& input_mode;
    std::int32_t& dpi_scale;
    std::int32_t& menu_width;
    std::int32_t& menu_height;
    std::array<std::uint32_t, 128>& script_hashes;
    std::uint32_t& optional_value;
    std::array<std::int32_t, 21>& menu_controls;
    std::uint8_t& flag;
    const std::byte* native_owner() const noexcept {
        return native_owner_;
    }
};

// ScriptOwner and RootMenu precede the native settings allocation. Production
// borrows its canonical publication and resolves it at each real use; creating
// an API/menu context neither allocates settings nor freezes an absent owner.
class SettingsStateBinding {
public:
    SettingsStateBinding(SettingsPersistenceState* standalone = nullptr) noexcept
        : standalone_(standalone) {}
    SettingsStateBinding(SettingsPersistenceState& standalone) noexcept
        : standalone_(&standalone) {}
    SettingsStateBinding(PreservedConfigStorage& storage) noexcept : storage_(&storage) {}
    SettingsPersistenceState* get() const;
    SettingsPersistenceState& require() const;

private:
    SettingsPersistenceState* standalone_ = nullptr;
    PreservedConfigStorage* storage_ = nullptr;
};

// 433EEC8B updates the persistent owner and serializes its selected records.
// A zero optional_value omits that record and preserves its stored value.
bool serialize_settings_snapshot(SettingsPersistenceState& state,
                                 const SettingsPersistenceInputs& inputs,
                                 std::uint32_t optional_value, std::uint32_t excluded_script,
                                 ConfigBytes& output);
std::uint32_t settings_module_hash(std::u16string_view module) noexcept;

class SettingsPersistenceBackend {
public:
    virtual ~SettingsPersistenceBackend() = default;
    using ModuleVisitor = bool (*)(void*, std::u16string_view);
    // false from the visitor requests the LdrEnumerateLoadedModules stop flag.
    virtual void enumerate_loaded_modules(ModuleVisitor, void*) = 0;
    // Native ntdll!RtlWriteRegistryValue boundary. Names are terminated native
    // or retained standalone strings; their pointer is passed without copying.
    // The return status is ignored.
    virtual void write_registry_value(std::uint32_t relative_to, std::u16string_view path,
                                      std::u16string_view value_name, std::uint32_t type,
                                      std::span<const std::byte> data) = 0;
};

class SettingsPersistence {
public:
    using Capture = std::function<SettingsPersistenceInputs()>;
    SettingsPersistence(SettingsPersistenceBackend& backend, SettingsPersistenceState& state,
                        std::u16string_view value_name, Capture capture);
    SettingsPersistence(SettingsPersistenceBackend&, PreservedConfigStorage&, Capture);
    // Complete 4340B9C9 boundary: module check, snapshot, user Software write.
    // false means the native module gate blocked the write or metadata is invalid.
    bool persist(std::uint32_t optional_value = 0, std::uint32_t excluded_script = 0);

private:
    SettingsPersistenceBackend& backend_;
    SettingsStateBinding state_;
    std::u16string value_name_;
    PreservedConfigStorage* preserved_ = nullptr;
    Capture capture_;
};

} // namespace skeet
