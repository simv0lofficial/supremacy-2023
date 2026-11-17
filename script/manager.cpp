#include "skeet/script/manager.hpp"
#include "skeet/script/ui.hpp"
#include <algorithm>
#include <stdexcept>

namespace skeet::script {
ScriptManager::ScriptManager(Bridge& bridge, ShutdownState& shutdown, UiBindings& ui,
                             ScriptRuntime& runtime, std::span<BuiltinModule> builtins,
                             ScriptFileSystem& files, SettingsStateBinding settings,
                             SettingsPersistence* persistence) noexcept
    : bridge_(bridge), shutdown_(shutdown), ui_(ui), runtime_(runtime), builtins_(builtins),
      files_(files), settings_(settings), persistence_(persistence) {
    runtime_.builtins = builtins_;
    runtime_.ui_bindings = &ui_;
    shutdown_.ui = &ui_;
}
bool ScriptManager::ready() const noexcept {
    const std::lock_guard lock(bridge_.vm_mutex);
    return bridge_.state != nullptr;
}
bool ScriptManager::loaded(std::u16string_view name) const {
    return script_is_loaded(name, builtins_, loaded_files_);
}
void ScriptManager::refresh() {
    records_ = discover_scripts(builtins_, files_, settings_.get());
}
void ScriptManager::startup_changed() {
    auto* settings = settings_.get();
    if (!settings)
        return;
    settings->script_hashes = startup_script_hashes(records_);
    (void)persist_settings();
}
bool ScriptManager::persist_settings() {
    if (!persistence_)
        throw std::logic_error("Script settings persistence is not bound");
    return persistence_->persist(0, 0);
}
ConfigBytes ScriptManager::save_ui_values() const {
    return save_script_ui_values(ui_);
}
void ScriptManager::restore_ui_values(std::span<const std::byte> bytes) {
    restore_script_ui_values(ui_, bytes, preserve_hotkey_toggle);
}
void ScriptManager::shutdown() {
    const std::lock_guard lock(bridge_.vm_mutex);
    notify_shutdown(bridge_);
    destroy_runtime(shutdown_);
}
void ScriptManager::initialize() { //4335BBBF
    const std::lock_guard lock(bridge_.vm_mutex);
    notify_shutdown(bridge_);
    const auto saved = save_ui_values();
    destroy_runtime(shutdown_);
    initialize_modules(loaded_files_);
    if (ready())
        restore_ui_values(saved);
}
bool ScriptManager::load(std::size_t index) {
    return index < records_.size() && load_name(records_[index].name);
}
bool ScriptManager::unload(std::size_t index) {
    return index < records_.size() && unload_name(records_[index].name);
}
} // namespace skeet::script
