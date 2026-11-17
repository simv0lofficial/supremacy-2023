// Menu Component: checkbox
#include "skeet/client_menu.hpp"
#include "config_binding.hpp"
#include <algorithm>
#include <stdexcept>


// ============================================================================
// Subcomponent: bound_menu_controls.cpp
// ============================================================================

namespace skeet::ui {
namespace menu_detail {
ConfigDescriptor& descriptor(ConfigRegistries& settings, std::uint32_t hash) {
    auto* link = settings.descriptors.next ? find_config(settings.descriptors, hash) : nullptr;
    if (!link)
        throw std::invalid_argument("Menu requires an existing configuration descriptor");
    return static_cast<ConfigDescriptor&>(*link);
}
void require_settings(ConfigRegistries& settings, std::span<const Requirement> requirements) {
    for (const auto& required : requirements)
        if (descriptor(settings, required.hash).type != required.type)
            throw std::invalid_argument("Menu configuration descriptor has the wrong native type");
}
}

Widget* BoundMenuControls::find(std::uint32_t hash) const noexcept {
    for (const auto& binding : bindings_)
        if (binding.config_hash == hash)
            return binding.widget;
    return nullptr;
}
void BoundMenuControls::detach(Category& owner) noexcept {
    std::erase_if(owner.elements, [this](const auto* widget) {
        return std::any_of(bindings_.begin(), bindings_.end(),
                           [widget](const auto& binding) { return binding.widget == widget; });
    });
}

} // namespace skeet::ui
