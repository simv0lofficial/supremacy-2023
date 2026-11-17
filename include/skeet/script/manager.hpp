#pragma once
#include "skeet/script/initialize.hpp"
#include "skeet/script/shutdown.hpp"
#include "skeet/config_storage/settings_persistence.hpp"
#include "skeet/config_storage/load_state.hpp"
#include <array>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <vector>

namespace skeet::ui {
class ScriptsMenu;
}

namespace skeet::script {
class UiBindings;
struct ScriptRecord {
    std::uint64_t modified_time = 0;
    std::uint8_t startup = 0;
    std::u16string name;
};
struct ScriptDirectoryEntry {
    std::u16string filename;
    std::uint64_t last_write_time = 0; // native FILE_DIRECTORY_INFORMATION, 100ns since1601
};
// The only discovery boundary is the OS directory service. The system backend
// performs the native NT directory query; no client discovery helper is injected.
class ScriptFileSystem {
public:
    virtual ~ScriptFileSystem() = default;
    virtual std::optional<std::u16string> current_directory() = 0;
    virtual std::vector<ScriptDirectoryEntry> enumerate_lua(std::u16string_view directory) = 0;
};
ScriptFileSystem& system_script_files() noexcept;
std::vector<ScriptRecord> discover_scripts(std::span<const BuiltinModule>, ScriptFileSystem&,
                                           const SettingsPersistenceState*);
bool script_is_loaded(std::u16string_view, std::span<const BuiltinModule>,
                      const std::set<std::u16string>&);
ConfigBytes save_script_ui_values(const UiBindings&);
void restore_script_ui_values(UiBindings&, std::span<const std::byte>,
                              const bool* preserve_hotkey_toggle);
std::uint32_t script_name_hash(std::u16string_view) noexcept; // 4334ADB0
std::uint32_t script_name_hash(std::string_view) noexcept; // 43359C70
std::uint64_t script_file_time(std::uint64_t windows_time) noexcept;
std::array<std::uint32_t, 128> startup_script_hashes(std::span<const ScriptRecord>,
                                                     std::uint32_t excluded = 0) noexcept;

class ScriptManager {
public:
    ScriptManager(Bridge&, ShutdownState&, UiBindings&, ScriptRuntime&, std::span<BuiltinModule>,
                  ScriptFileSystem&, SettingsStateBinding settings = {},
                  SettingsPersistence* persistence = nullptr) noexcept;
    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;
    std::span<ScriptRecord> records() noexcept {
        return records_;
    }
    std::span<const ScriptRecord> records() const noexcept {
        return records_;
    }
    std::set<std::u16string>& loaded_files() noexcept {
        return loaded_files_;
    }
    const std::set<std::u16string>& loaded_files() const noexcept {
        return loaded_files_;
    }
    bool ready() const noexcept;
    bool loaded(std::u16string_view name) const;
    void initialize(); //4335BBBF: saveUI, shutdown, recreate active modules, restoreUI
    void refresh(); // Model discovery utility;4336D863's retention branch belongs to ScriptsMenu.
    bool load(std::size_t index); //43419B27 transaction and rollback
    bool unload(std::size_t index); //433DF341
    void startup_changed(); //433E7D9D then SettingsPersistence::persist(0,0)
    bool persist_settings();
    void shutdown();
    void update(std::uint8_t& reload_requested); //4338174B/433882DB
    std::span<const std::byte> current_config() const noexcept {
        return shutdown_.current_config;
    }
    ShutdownState& shutdown_state() noexcept {
        return shutdown_;
    }
    // Shared native byte43475A98, also used by category loads and preset resets.
    const bool* preserve_hotkey_toggle = &config_loading_flag();

private:
    friend class ui::ScriptsMenu; //4341AABA owns startup refresh/load/option ordering.
    void initialize_modules(std::set<std::u16string>&);
    int load_module(std::string_view);
    ConfigBytes save_ui_values() const;
    void restore_ui_values(std::span<const std::byte>);
    bool load_name(std::u16string_view);
    bool unload_name(std::u16string_view);
    Bridge& bridge_;
    ShutdownState& shutdown_;
    UiBindings& ui_;
    ScriptRuntime& runtime_;
    std::span<BuiltinModule> builtins_;
    ScriptFileSystem& files_;
    SettingsStateBinding settings_;
    SettingsPersistence* persistence_;
    std::set<std::u16string> loaded_files_;
    std::vector<ScriptRecord> records_;
    ConfigBytes current_ui_config_;
};
} // namespace skeet::script
