#include "skeet/script/manager.hpp"
#include "skeet/script/ui.hpp"
#include <stdexcept>

namespace skeet::script {
ConfigBytes save_script_ui_values(const UiBindings& ui) { //4336F8B2
    ConfigBytes output;
    if (ui.records.empty())
        return output;
    initialize_config_stream(output);
    const auto count = ui.records.size();
    for (std::size_t index = 0; index < count; ++index) {
        const auto& binding = ui.records[index];
        if (!binding.borrowed && binding.configuration)
            if (!serialize_config_value(*binding.configuration, output))
                throw std::logic_error("Script UI configuration has an unsupported native type");
    }
    return output;
}
void restore_script_ui_values(UiBindings& ui, std::span<const std::byte> bytes,
                              const bool* preserve_hotkey_toggle) { //4337EABC
    if (bytes.empty())
        return;
    const auto count = ui.records.size();
    for (std::size_t index = 0; index < count; ++index) {
        auto& binding = ui.records[index];
        if (binding.borrowed || !binding.configuration)
            continue;
        const bool hotkey = binding.configuration->type == ConfigType::hotkey;
        if (hotkey && !preserve_hotkey_toggle)
            throw std::logic_error("Script UI hotkey preservation byte is not bound");
        const bool changed = deserialize_config_value(*binding.configuration, bytes,
                                                      hotkey ? *preserve_hotkey_toggle : false);
        if (binding.widget) {
            binding.widget->config_loaded();
            if (changed)
                ui.records[index].widget->value_changed();
        }
    }
}
} // namespace skeet::script
