#include "skeet/script/config_api.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/script/shutdown.hpp"
#include <array>
#include <cstring>
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*export_context)(void*, int) = nullptr;
constexpr int globals = -10002;
constexpr int first_upvalue = -10003;
constexpr const char* restriction_warning = "Attempted to load untrusted config in a valve server.";

std::vector<ConfigDescriptor*> tab_descriptors(ui::Tab& tab, ConfigLink& registry) {
    std::vector<ConfigDescriptor*> descriptors;
    for (auto* category : std::span(tab.categories)) {
        const auto selected = ui::collect_category_config(*category, registry);
        descriptors.insert(descriptors.end(), selected.begin(), selected.end());
    }
    return descriptors;
}
bool save_tab(ui::MenuWindow& window, ConfigLink& registry, std::string_view name,
              ConfigBytes& output, std::span<const std::byte> baseline_a,
              std::span<const std::byte> baseline_b) { // 0x4334D0EB
    auto* tab = ui::find_config_tab(window, ui::category_name_hash(name));
    if (!tab)
        return false;
    const auto descriptors = tab_descriptors(*tab, registry);
    initialize_config_stream(output);
    for (const auto* descriptor : descriptors) {
        const auto key = config_record_key(*descriptor);
        const auto first = find_config_record(baseline_a, key);
        if (first && config_value_equals(*descriptor, first->value))
            continue;
        const auto second = find_config_record(baseline_b, key);
        if (second && config_value_equals(*descriptor, second->value))
            continue;
        if (!serialize_config_value(*descriptor, output))
            throw std::logic_error("Unsupported Lua tab configuration descriptor");
    }
    return true;
}
bool load_tab(ui::MenuWindow& window, ConfigLink& registry, std::string_view name,
              std::span<const std::byte> stream, std::span<const std::byte> baseline_a,
              std::span<const std::byte> baseline_b) { // 0x4332ADAC
    auto* tab = ui::find_config_tab(window, ui::category_name_hash(name));
    if (!tab)
        return false;
    auto descriptors = tab_descriptors(*tab, registry);
    ui::load_category_config_values(descriptors, stream, baseline_a, baseline_b);
    tab->config_loaded();
    return true;
}
}

void ConfigModule::bind_current_config(ShutdownState& state) {
    if (!bindings_.baseline_b.is_unbound() && !bindings_.baseline_b.is_script_state(state))
        throw std::invalid_argument("Lua configuration baseline belongs to another script owner");
    bindings_.baseline_b = ConfigStreamView::script_state(state);
}
void ConfigModule::validate_streams(ConfigStreamView baseline_a, const ShutdownState& state) const {
    if (!bindings_.baseline_a.same_source(baseline_a) ||
        !bindings_.baseline_b.is_script_state(state))
        throw std::invalid_argument(
            "Lua configuration requires its canonical baseline and script owner streams");
}
bool ConfigModule::restricted() const {
    const auto& environment = bindings_.restriction;
    return engine::invoke<bool>(bindings_.engine, 26) && *environment.angle_guard == 0 &&
           environment.shared_player && *environment.shared_player &&
           client::command_roll_blocked(environment);
}
void ConfigModule::restore_restricted() {
    if (!restricted())
        return;
    if (!bridge_->report_error)
        throw std::logic_error("Restricted Lua configuration requires its console error sink");
    bridge_->report_error(bridge_->error_context, restriction_warning);
    bindings_.presets.restore_default_values();
}
void ConfigModule::load_full(std::span<const std::byte> stream) { // 0x433ED806
    if (stream.empty())
        return;
    auto& services = bindings_.services;
    services.before_baseline_load();
    deserialize_config_registry(bindings_.configuration.descriptors, bindings_.baseline_a.bytes(),
                                bindings_.presets.preserve_hotkey_toggle);
    services.after_baseline_load();
    deserialize_config_registry(bindings_.configuration.descriptors, stream,
                                bindings_.presets.preserve_hotkey_toggle);
    if (restricted()) {
        if (!bridge_->report_error)
            throw std::logic_error("Restricted Lua configuration requires its console error sink");
        bridge_->report_error(bridge_->error_context, restriction_warning);
        bindings_.presets.restore_default_values();
    } else {
        services.after_values_load();
        services.loaded_event();
    }
}
void ConfigModule::import_full(std::span<const std::byte> stream) { // 0x43347A96
    if (stream.empty())
        return;
    auto& services = bindings_.services;
    auto& presets = bindings_.presets;
    presets.active_hash = 0;
    services.before_baseline_load();
    deserialize_config_registry(bindings_.configuration.descriptors, bindings_.baseline_a.bytes(),
                                bindings_.presets.preserve_hotkey_toggle);
    services.after_baseline_load();
    deserialize_config_registry(bindings_.configuration.descriptors, stream,
                                presets.preserve_hotkey_toggle);
    if (restricted()) {
        if (!bridge_->report_error)
            throw std::logic_error("Restricted Lua configuration requires its console error sink");
        bridge_->report_error(bridge_->error_context, restriction_warning);
        presets.active_hash = 0;
        presets.restore_default_values();
        presets.update_markers();
    } else {
        services.after_values_load();
    }
    presets.update_markers();
    services.loaded_event();
}

int ConfigModule::load(void* state) {
    const auto& api = *bridge_->api;
    const auto count = api.gettop(state);
    if (count < 1)
        return 0;
    std::size_t length = 0;
    const auto* name = api.checklstring(state, 1, &length);
    if (!name || !*name || !length)
        return 0;
    const auto stream = bindings_.named.load(std::string_view(name, std::strlen(name)));
    if (stream.empty())
        return 0;
    if (count == 1) {
        load_full(stream);
    } else if (count == 2) {
        const auto* tab = api.checklstring(state, 2, &length);
        if (!tab || !length)
            return 0;
        load_tab(bindings_.window, bindings_.configuration.descriptors, tab, stream,
                 bindings_.baseline_a.bytes(), bindings_.baseline_b.bytes());
    } else if (count == 3) {
        const auto* tab = api.checklstring(state, 2, &length);
        if (!tab || !length)
            return 0;
        const auto* category = api.checklstring(state, 3, &length);
        if (!category || !length)
            return 0;
        ui::load_category_config(bindings_.window, bindings_.configuration.descriptors, tab,
                                 category, stream, bindings_.baseline_a.bytes(),
                                 bindings_.baseline_b.bytes());
    } else {
        return 0;
    }
    restore_restricted();
    return 0;
}

int ConfigModule::export_values(void* state) {
    const auto& api = *bridge_->api;
    const auto count = api.gettop(state);
    ConfigBytes stream;
    if (count == 0) {
        const auto text =
            save_config_export(bindings_.configuration.descriptors, bindings_.baseline_a.bytes());
        if (!text)
            throw std::logic_error("Unsupported Lua configuration descriptor");
        api.pushlstring(state, text->data(), text->size());
        return 1;
    }
    std::size_t length = 0;
    if (count == 1) {
        const auto* tab = api.checklstring(state, 1, &length);
        if (!tab || !length) {
            api.pushnil(state);
            return 1;
        }
        save_tab(bindings_.window, bindings_.configuration.descriptors, tab, stream,
                 bindings_.baseline_a.bytes(), bindings_.baseline_b.bytes());
    } else if (count == 2) {
        const auto* tab = api.checklstring(state, 1, &length);
        if (!tab || !length) {
            api.pushnil(state);
            return 1;
        }
        const auto* category = api.checklstring(state, 2, &length);
        if (!category || !length) {
            api.pushnil(state);
            return 1;
        }
        ui::save_category_config(bindings_.window, bindings_.configuration.descriptors, tab,
                                 category, stream, bindings_.baseline_a.bytes(),
                                 bindings_.baseline_b.bytes());
    } else {
        api.pushnil(state);
        return 1;
    }
    const auto text = encode_config_export(stream);
    api.pushlstring(state, text.data(), text.size());
    return 1;
}

int ConfigModule::import_values(void* state) {
    const auto& api = *bridge_->api;
    const auto count = api.gettop(state);
    if (count < 1)
        return 0;
    std::size_t length = 0;
    const auto* encoded = api.checklstring(state, 1, &length);
    if (!encoded || !*encoded || !length)
        return 0;
    if (count == 1) {
        const auto stream = decode_config_export({encoded, length});
        import_full(stream);
    } else if (count == 2) {
        const auto encoded_length = length;
        const auto* tab = api.checklstring(state, 2, &length);
        if (!tab || !length)
            return 0;
        const auto stream = decode_config_export({encoded, encoded_length});
        load_tab(bindings_.window, bindings_.configuration.descriptors, tab, stream,
                 bindings_.baseline_a.bytes(), bindings_.baseline_b.bytes());
    } else if (count == 3) {
        const auto encoded_length = length;
        const auto* tab = api.checklstring(state, 2, &length);
        if (!tab || !length)
            return 0;
        const auto* category = api.checklstring(state, 3, &length);
        if (!category || !length)
            return 0;
        const auto stream = decode_config_export({encoded, encoded_length});
        ui::load_category_config(bindings_.window, bindings_.configuration.descriptors, tab,
                                 category, stream, bindings_.baseline_a.bytes(),
                                 bindings_.baseline_b.bytes());
    }
    return 0;
}

int ConfigModule::load_entry(void* state) {
    return static_cast<ConfigModule*>(export_context(state, first_upvalue))->load(state);
}
int ConfigModule::export_entry(void* state) {
    return static_cast<ConfigModule*>(export_context(state, first_upvalue))->export_values(state);
}
int ConfigModule::import_entry(void* state) {
    return static_cast<ConfigModule*>(export_context(state, first_upvalue))->import_values(state);
}
void ConfigModule::register_exports(Bridge& bridge) {
    if (!bridge.api || !bridge.state || !bindings_.engine || !bindings_.restriction.angle_guard)
        throw std::logic_error("Lua configuration exports require initialized runtime bindings");
    bridge_ = &bridge;
    const auto& api = *bridge.api;
    export_context = api.touserdata;
    api.createtable(bridge.state, 0, 3);
    for (const auto& function : std::array<std::pair<const char*, LuaApi::CFunction>, 3>{
             {{"load", load_entry}, {"export", export_entry}, {"import", import_entry}}}) {
        api.pushlightuserdata(bridge.state, this);
        api.pushcclosure(bridge.state, function.second, 1);
        api.setfield(bridge.state, -2, function.first);
    }
    api.setfield(bridge.state, globals, "config");
}

} // namespace skeet::script
