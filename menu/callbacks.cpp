// Menu Component: callbacks
#include "skeet/menu/anti_aim_extra_menu.hpp"
#include "config_binding.hpp"
#include <algorithm>
#include "skeet/menu/anti_aim_menu.hpp"
#include "skeet/anti_aim/config.hpp"
#include <stdexcept>
#include "skeet/menu/category_config.hpp"
#include "skeet/config_storage/load_state.hpp"
#include "skeet/color_picker.hpp"
#include "skeet/combobox.hpp"
#include "skeet/hotkey_widget.hpp"
#include "skeet/listbox.hpp"
#include "skeet/multiselect.hpp"
#include "skeet/menu/config_menu.hpp"
#include "skeet/config_storage/presets.hpp"
#include <bit>
#include <immintrin.h>
#include "skeet/client_menu.hpp"
#include "skeet/legit/config.hpp"
#include "skeet/menu/legit_menu.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/misc/rebuy_fix.hpp"
#include "skeet/misc/quick_switch.hpp"
#include "skeet/misc/reveal_ranks.hpp"
#include "skeet/misc/settings_runtime.hpp"
#include "skeet/menu/players_menu.hpp"
#include "skeet/menu/rage_clipboard.hpp"
#include <windows.h>
#include <cstring>
#include "skeet/menu/rage_menu.hpp"
#include "skeet/menu/rage_weapon_groups.hpp"
#include "skeet/menu/rage_weapon_selector.hpp"
#include "skeet/menu_tree.hpp"
#include "skeet/scrollbar_render.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/rage/group_storage.hpp"
#include "skeet/menu/skins_menu.hpp"
#include "skeet/skins/native_catalog.hpp"
#include "skeet/script/owner.hpp"
#include "skeet/render/icon_cache.hpp"
#include "skeet/render/backend_container.hpp"
#include "skeet/render/menu_window_assets.hpp"
#include "skeet/render/dpi_bridge.hpp"
#include "skeet/input/runtime.hpp"
#include <optional>
#include "skeet/engine/interface_call.hpp"
#include "skeet/menu/scripts_menu.hpp"
#include "skeet/script/manager.hpp"
#include <utility>
#include <charconv>
#include <ctime>
#include "skeet/menu/settings_menu_storage.hpp"
#include "skeet/skins/widget_setters.hpp"
#include "skeet/skins/catalog_sort.hpp"
#include "skeet/menu/startup_config_path.hpp"
#include "skeet/menu/startup_window_name.hpp"
#include "skeet/config_storage/preserved.hpp"
#include "skeet/script/database.hpp"
#include <array>
#include <string>
#include <winternl.h>
#include "skeet/menu/startup_data.hpp"
#include "skeet/menu/startup_registry.hpp"
#include "skeet/engine/owner_allocation.hpp"


// ============================================================================
// Subcomponent: anti_aim_fakelag_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
FakeLagMenu::FakeLagMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Fake lag", {12, 12, 0, 12}, 13, true, identities) {
    constexpr Requirement fakelag_required[] = {{0xF9BBEC7Bu, ConfigType::boolean},
                                        {0x27D21BD4u, ConfigType::hotkey},
                                        {0x2A71D5FCu, ConfigType::integer},
                                        {0xCA9768E6u, ConfigType::integer},
                                        {0xF17FEF11u, ConfigType::integer}};
    require_settings(settings, fakelag_required);
    tab.categories.push_back(&category_);
    add<Checkbox>(category_, 0xF9BBEC7Bu, u"Enabled", value<BooleanConfig>(settings, 0xF9BBEC7Bu),
                  identities);
    add<HotkeyWidget>(category_, 0x27D21BD4u, value<HotkeyConfig>(settings, 0x27D21BD4u), true,
                      identities);
    auto& amount = add<ComboBox>(category_, 0x2A71D5FCu, u"Amount",
                                 value<IntegerConfig>(settings, 0x2A71D5FCu), false, identities);
    amount.set_options({{u"Dynamic", 0}, {u"Maximum", 2}, {u"Fluctuate", 3}});
    add<Slider>(category_, 0xCA9768E6u, u"Variance", 0, 100,
                value<IntegerConfig>(settings, 0xCA9768E6u), true, 0x25u, 1.0f, identities);
    limit_ = &add<Slider>(category_, 0xF17FEF11u, u"Limit", 1, 15,
                          value<IntegerConfig>(settings, 0xF17FEF11u), true, 0u, 1.0f, identities);
}
FakeLagMenu::~FakeLagMenu() {
    std::erase(tab_.categories, &category_);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: anti_aim_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement settings_required[] = {
    {0x96C0EF29u, ConfigType::boolean}, {0x6D5F1172u, ConfigType::integer},
    {0x01DE265Eu, ConfigType::integer}, {0x1D43759Bu, ConfigType::integer},
    {0xF2D408F3u, ConfigType::integer}, {0xFE49974Du, ConfigType::integer_array},
    {0x177D7470u, ConfigType::integer}, {0xDCA4B692u, ConfigType::integer},
    {0x4D6D88C6u, ConfigType::integer}, {0x8DFD54BDu, ConfigType::integer_array},
    {0x2716923Fu, ConfigType::boolean}, {0x8C526C4Fu, ConfigType::boolean},
    {0x86C04DC4u, ConfigType::boolean}, {0x697E8050u, ConfigType::hotkey},
    {0xA1DFC38Du, ConfigType::integer},
};
}
AntiAimMenu::AntiAimMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Anti-aimbot angles", {0, 12, 0, 24}, 10, true, identities) {
    require_settings(settings, settings_required);
    if (descriptor(settings, 0xFE49974Du).serialized_size != 24 ||
        descriptor(settings, 0x8DFD54BDu).serialized_size != 16)
        throw std::invalid_argument("Anti-aim requires its six-yaw and four-body offset arrays");
    yaw_offsets_ = value<anti_aim::YawOffsetsConfig>(settings, 0xFE49974Du)->data();
    body_offsets_ = value<anti_aim::BodyYawOffsetsConfig>(settings, 0x8DFD54BDu)->data();
    storage_.reserve(15);
    bindings_.reserve(15);
    tab.categories.push_back(&category_);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto combobox = [&](std::uint32_t hash, std::u16string_view label) -> ComboBox& {
        return add<ComboBox>(category_, hash, label, value<IntegerConfig>(settings, hash), false,
                             identities);
    };
    auto slider = [&](std::uint32_t hash, std::int32_t* setting, int minimum,
                      int maximum) -> Slider& {
        return add<Slider>(category_, hash, u"", minimum, maximum, setting, true, 0xB0u, 1.0f,
                           identities);
    };
    checkbox(0x96C0EF29u, u"Enabled");
    auto& pitch = combobox(0x6D5F1172u, u"Pitch");
    pitch.set_options({{u"Off", 0},
                       {u"Default", 1},
                       {u"Up", 5},
                       {u"Down", 2},
                       {u"Minimal", 3},
                       {u"Random", 4},
                       {u"Custom", 6}});
    pitch_offset_ = &slider(0x01DE265Eu, value<IntegerConfig>(settings, 0x01DE265Eu), -89, 89);
    pitch_offset_->set_visible(false);
    combobox(0x1D43759Bu, u"Yaw base").set_options({{u"Local view", 0}, {u"At targets", 1}});
    yaw_ = &combobox(0xF2D408F3u, u"Yaw");
    yaw_->set_options(
        {{u"Off", 0}, {u"180", 1}, {u"Spin", 3}, {u"Static", 5}, {u"180 Z", 2}, {u"Crosshair", 4}});
    yaw_offset_ = &slider(0xFE49974Du, yaw_offsets_, -180, 180);
    yaw_offset_->set_visible(false);
    jitter_ = &combobox(0x177D7470u, u"Yaw jitter");
    jitter_->set_options(
        {{u"Off", 0}, {u"Offset", 1}, {u"Center", 2}, {u"Random", 3}, {u"Skitter", 4}});
    jitter_->set_visible(false);
    jitter_offset_ = &slider(0xDCA4B692u, value<IntegerConfig>(settings, 0xDCA4B692u), -180, 180);
    jitter_offset_->set_visible(false);
    auto& body = combobox(0x4D6D88C6u, u"Body yaw");
    body.set_options({{u"Off", 0}, {u"Opposite", 1}, {u"Jitter", 2}, {u"Static", 3}});
    body_offset_ = &slider(0x8DFD54BDu, body_offsets_, -180, 180);
    body_offset_->set_visible(false);
    freestanding_body_ = &checkbox(0x2716923Fu, u"Freestanding body yaw");
    freestanding_body_->set_visible(false);
    checkbox(0x8C526C4Fu, u"Edge yaw");
    checkbox(0x86C04DC4u, u"Freestanding");
    add<HotkeyWidget>(category_, 0x697E8050u, value<HotkeyConfig>(settings, 0x697E8050u), true,
                      identities);
    add<Slider>(category_, 0xA1DFC38Du, u"Roll", -45, 45,
                value<IntegerConfig>(settings, 0xA1DFC38Du), true, 0xB0u, 1.0f, identities);
    pitch.callbacks.push_back({this, pitch_changed}); // 0x433CF43F
    yaw_->callbacks.push_back({this, yaw_changed}); // 0x433CC833
    jitter_->callbacks.push_back({this, jitter_changed}); // 0x433598F4
    body.callbacks.push_back({this, body_changed}); // 0x433C65CC
}
AntiAimMenu::~AntiAimMenu() {
    std::erase(tab_.categories, &category_);
}
anti_aim::MenuDependencies AntiAimMenu::dependencies() {
    return {*pitch_offset_,
            *yaw_offset_,
            *jitter_,
            *jitter_offset_,
            *body_offset_,
            *freestanding_body_,
            std::span<std::int32_t, 6>(yaw_offsets_, 6),
            std::span<std::int32_t, 4>(body_offsets_, 4)};
}
void AntiAimMenu::pitch_changed(void* context, Widget& source) {
    auto bindings = static_cast<AntiAimMenu*>(context)->dependencies();
    anti_aim::pitch_changed(bindings, static_cast<ComboBox&>(source));
}
void AntiAimMenu::yaw_changed(void* context, Widget& source) {
    auto bindings = static_cast<AntiAimMenu*>(context)->dependencies();
    anti_aim::yaw_changed(bindings, static_cast<ComboBox&>(source));
}
void AntiAimMenu::jitter_changed(void* context, Widget& source) {
    auto& self = *static_cast<AntiAimMenu*>(context);
    auto bindings = self.dependencies();
    anti_aim::jitter_changed(bindings, static_cast<ComboBox&>(source), *self.yaw_);
}
void AntiAimMenu::body_changed(void* context, Widget& source) {
    auto bindings = static_cast<AntiAimMenu*>(context)->dependencies();
    anti_aim::body_yaw_changed(bindings, static_cast<ComboBox&>(source));
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: anti_aim_other_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
AntiAimOtherMenu::AntiAimOtherMenu(Tab& tab, ConfigRegistries& settings,
                                   WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Other", {12, 12, 12, 12}, 13, true, identities) {
    constexpr Requirement anti_aim_other_required[] = {
        {0x4DF394FFu, ConfigType::boolean}, {0x7293DE2Au, ConfigType::hotkey},
        {0x8F8A4366u, ConfigType::integer}, {0xB2467427u, ConfigType::boolean},
        {0x8F4F3C91u, ConfigType::hotkey},  {0x8494FC0Du, ConfigType::boolean},
        {0xAE772257u, ConfigType::hotkey}};
    require_settings(settings, anti_aim_other_required);
    tab.categories.push_back(&category_);
    add<Checkbox>(category_, 0x4DF394FFu, u"Slow motion",
                  value<BooleanConfig>(settings, 0x4DF394FFu), identities);
    add<HotkeyWidget>(category_, 0x7293DE2Au, value<HotkeyConfig>(settings, 0x7293DE2Au), true,
                      identities);
    auto& legs = add<ComboBox>(category_, 0x8F8A4366u, u"Leg movement",
                               value<IntegerConfig>(settings, 0x8F8A4366u), false, identities);
    legs.set_options({{u"Off", 0}, {u"Always slide", 1}, {u"Never slide", 2}});
    auto& on_shot = add<Checkbox>(category_, 0xB2467427u, u"On shot anti-aim",
                                  value<BooleanConfig>(settings, 0xB2467427u), identities);
    on_shot.color = 0xBB82F0F0u;
    add<HotkeyWidget>(category_, 0x8F4F3C91u, value<HotkeyConfig>(settings, 0x8F4F3C91u), true,
                      identities);
    auto& fake = add<Checkbox>(category_, 0x8494FC0Du, u"Fake peek",
                               value<BooleanConfig>(settings, 0x8494FC0Du), identities);
    fake.color = 0xBB82F0F0u;
    add<HotkeyWidget>(category_, 0xAE772257u, value<HotkeyConfig>(settings, 0xAE772257u), true,
                      identities);
}
AntiAimOtherMenu::~AntiAimOtherMenu() {
    std::erase(tab_.categories, &category_);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: category_config.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::uint32_t lookup_hash(std::u16string_view name) noexcept {
    name = name.substr(0, name.find(u'\0'));
    // 0x433FAB07 decodes into 64 UTF16 units. It clears the destination before
    // rejecting a source whose content plus terminator exceeds that buffer.
    return category_name_hash(name.size() < 64 ? name : std::u16string_view{});
}
const void* widget_value(const Widget& widget) {
    switch (widget.kind) {
    case 3:
        return static_cast<const Checkbox&>(widget).value;
    case 4:
        return static_cast<const ComboBox&>(widget).value;
    case 5:
        return static_cast<const Slider&>(widget).value;
    case 6:
        return static_cast<const Listbox&>(widget).value;
    case 9: {
        auto* hotkey = static_cast<const HotkeyWidget&>(widget).value;
        // The native collector adds 16 even to a zero hotkey pointer. Preserve
        // that nonzero query: nullptr has distinct meaning for tag-12 entries.
        return hotkey ? &hotkey->binding : reinterpret_cast<const void*>(std::uintptr_t(16));
    }
    case 10:
        return nullptr; // Textbox's 0x5C slot is zero from its constructor.
    case 12:
        return static_cast<const ColorPicker&>(widget).value;
    case 13:
        return static_cast<const MultiSelect&>(widget).value;
    default:
        throw std::logic_error("Unsupported widget kind in category configuration");
    }
}
}
std::uint32_t category_name_hash(std::string_view name) noexcept { // 0x4334DEFF
    std::uint32_t hash = 0x811c9dc5;
    for (const unsigned char character : name) {
        if (!character)
            break;
        const auto folded = character >= 'A' && character <= 'Z' ? character + 0x20u : character;
        hash = (hash ^ folded) * 0x1000193u;
    }
    return hash ^ config_stream_hash_mask;
}
std::uint32_t category_name_hash(std::u16string_view name) noexcept { // 0x4341CD7B
    std::uint32_t hash = 0x811c9dc5;
    for (const char16_t character : name) {
        if (!character)
            break;
        const auto folded = character >= u'A' && character <= u'Z' ? character + 0x20u : character;
        hash = (hash ^ folded) * 0x1000193u;
    }
    return hash ^ config_stream_hash_mask;
}
Tab* find_config_tab(MenuWindow& window, std::uint32_t hash) { // 0x4332B5BD
    for (auto* tab : std::span(window.tabs))
        if (lookup_hash(tab->name) == hash)
            return tab;
    return nullptr;
}
Category* find_config_category(Tab& tab, std::uint32_t hash) { // 0x433E9117
    for (auto* category : std::span(tab.categories))
        if (lookup_hash(category->name) == hash)
            return category;
    return nullptr;
}
std::vector<ConfigDescriptor*> collect_category_config(Category& category,
                                                       ConfigLink& registry) { // 0x433E8D40
    std::vector<ConfigDescriptor*> descriptors;
    for (auto* widget : std::span(category.elements)) {
        switch (widget->kind) {
        case 0:
        case 1:
        case 2:
        case 7:
        case 8:
        case 11:
        case 14:
            continue;
        }
        if (auto* descriptor = find_config_by_value(registry, widget_value(*widget)))
            descriptors.push_back(descriptor);
    }
    return descriptors;
}
bool category_config_loading() noexcept {
    return config_loading_flag();
}
void load_category_config_values(std::span<ConfigDescriptor*> descriptors,
                                 std::span<const std::byte> selected,
                                 std::span<const std::byte> baseline_a,
                                 std::span<const std::byte> baseline_b) { // 0x43369B2C
    auto& loading = config_loading_flag();
    loading = true;
    for (const auto stream : {baseline_a, baseline_b, selected})
        if (!stream.empty())
            for (auto* descriptor : descriptors)
                deserialize_config_value(*descriptor, stream, loading);
    loading = false;
}
bool save_category_config(MenuWindow& window, ConfigLink& registry, std::string_view tab_name,
                          std::string_view category_name, ConfigBytes& output,
                          std::span<const std::byte> baseline_a,
                          std::span<const std::byte> baseline_b) { // 0x43381B62
    auto* tab = find_config_tab(window, category_name_hash(tab_name));
    auto* category = tab ? find_config_category(*tab, category_name_hash(category_name)) : nullptr;
    if (!category)
        return false;
    const auto descriptors = collect_category_config(*category, registry);
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
            throw std::logic_error("Unsupported category configuration descriptor");
    }
    return true;
}
bool load_category_config(MenuWindow& window, ConfigLink& registry, std::string_view tab_name,
                          std::string_view category_name, std::span<const std::byte> selected,
                          std::span<const std::byte> baseline_a,
                          std::span<const std::byte> baseline_b) { // 0x433B0879
    auto* tab = find_config_tab(window, category_name_hash(tab_name));
    auto* category = tab ? find_config_category(*tab, category_name_hash(category_name)) : nullptr;
    if (!category)
        return false;
    auto descriptors = collect_category_config(*category, registry);
    load_category_config_values(descriptors, selected, baseline_a, baseline_b);
    category->config_loaded();
    return true;
}
}

// ============================================================================
// Subcomponent: config_menu.cpp
// ============================================================================

namespace skeet::ui {

ConfigPresetsMenu::ConfigPresetsMenu(Tab& tab, WidgetIdentities& identities,
                                     ConfigPresetsManager& manager)
    : tab_(tab), category_(&tab, u"Presets", {0, 12, 0, 24}, 8, true, identities),
      manager_(manager) {
    if (manager_.menu_)
        throw std::logic_error("Preset manager already has a native configuration menu");
    storage_.reserve(9);
    bindings_.reserve(9);
    list_ = &add<Listbox>(category_, 0, u"Presets", Point{158, 126}, nullptr, false, identities);
    list_->callbacks.push_back({this, selection_changed});
    list_->enabled = true;
    hotkey_ = &add<HotkeyWidget>(category_, 0, nullptr, false, identities);
    hotkey_->set_visible(false);
    name_ = &add<Textbox>(category_, 0, identities);
    add<Button>(category_, 0, u"Load", Point{158, 28}, WidgetCallback{this, load}, identities);
    add<Button>(category_, 0, u"Save", Point{158, 28}, WidgetCallback{this, save}, identities);
    add<Button>(category_, 0, u"Delete", Point{158, 28}, WidgetCallback{this, erase}, identities);
    add<Button>(category_, 0, u"Reset", Point{158, 28}, WidgetCallback{this, reset}, identities);
    add<Button>(category_, 0, u"Import from clipboard", Point{158, 28},
                WidgetCallback{this, import_from_clipboard}, identities);
    add<Button>(category_, 0, u"Export to clipboard", Point{158, 28},
                WidgetCallback{this, export_to_clipboard}, identities);
    tab.categories.push_back(&category_);
    manager_.menu_ = this;
}

ConfigPresetsMenu::~ConfigPresetsMenu() {
    if (manager_.menu_ == this)
        manager_.menu_ = nullptr;
    std::erase(tab_.categories, &category_);
}

void ConfigPresetsMenu::read_name() {
    std::array<char16_t, 65> text{};
    const auto length = name_->copy_text(text);
    manager_.name.assign(text.data(), length);
}

void ConfigPresetsMenu::synchronize_from_manager() {
    const auto& entries = manager_.entries();
    std::vector<ListboxOption> options;
    options.reserve(entries.size());
    for (const auto& entry : entries)
        options.push_back({std::bit_cast<std::int32_t>(entry.marker), entry.name});
    list_->set_options(&options);
    synchronize_selection();
}

void ConfigPresetsMenu::update_markers() noexcept {
    for (std::size_t i = 0; i < list_->options.size(); ++i) {
        auto& option = list_->options[i];
        const auto marker =
            static_cast<std::uint32_t>(i) |
            (config_preset_hash(option.name) == manager_.active_hash ? 0x80000000u : 0u);
        option.value = std::bit_cast<std::int32_t>(marker);
    }
}

void ConfigPresetsMenu::synchronize_selection() {
    RegisteredHotkey* selected_key = nullptr;
    manager_.selected_index = std::bit_cast<std::uint32_t>(list_->selected_item);
    if (!list_->filtered_indices.empty()) {
        const auto& selected_name = list_->options[manager_.selected_index].name;
        // 0x4333C5F3 leaves the textbox untouched above 63 UTF-16 units.
        if (selected_name.size() <= 63) {
            while (name_->text_lock.exchange(1))
                _mm_pause();
            std::copy(selected_name.begin(), selected_name.end(), name_->text.begin());
            name_->text[selected_name.size()] = 0;
            name_->text_length.exchange(static_cast<std::uint32_t>(selected_name.size()));
            name_->text_lock.exchange(0);
        }
        const auto entries = manager_.entries();
        if (manager_.selected_index < entries.size())
            selected_key = entries[manager_.selected_index].hotkey;
    }
    read_name();
    hotkey_->value = selected_key;
    hotkey_->config_loaded();
    hotkey_->set_visible(hotkey_->value != nullptr);
    hotkey_->enabled = hotkey_->value != nullptr;
}

void ConfigPresetsMenu::refresh() {
    read_name();
    manager_.refresh();
}

void ConfigPresetsMenu::load_hotkey(RegisteredHotkey& hotkey) {
    const auto entries = manager_.entries();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].hotkey != &hotkey)
            continue;
        list_->set_selection(static_cast<std::uint32_t>(i));
        selection_changed(this, *list_);
        load(this, *list_);
        return;
    }
}

void ConfigPresetsMenu::selection_changed(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.synchronize_selection();
}

void ConfigPresetsMenu::load(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.load();
}
void ConfigPresetsMenu::save(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.save();
}
void ConfigPresetsMenu::erase(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.erase();
}
void ConfigPresetsMenu::reset(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.reset();
}
void ConfigPresetsMenu::import_from_clipboard(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.import_from_clipboard();
}
void ConfigPresetsMenu::export_to_clipboard(void* context, Widget&) {
    auto& self = *static_cast<ConfigPresetsMenu*>(context);
    self.read_name();
    self.manager_.export_to_clipboard();
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: effects_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement effects_settings[] = {
    {0x461D3721, ConfigType::boolean}, {0x623BD4F2, ConfigType::boolean},
    {0x3359E29C, ConfigType::boolean}, {0xFBD4768D, ConfigType::boolean},
    {0x171EED00, ConfigType::boolean}, {0xEAFCEAD9, ConfigType::boolean},
    {0xA8321370, ConfigType::boolean}, {0xD35F8540, ConfigType::boolean},
    {0x264E3E21, ConfigType::boolean}, {0x4CBB0E86, ConfigType::boolean},
    {0xB589F662, ConfigType::boolean}, {0x3A5285A6, ConfigType::boolean},
    {0xD54A1CBD, ConfigType::boolean}, {0xC82CBD3B, ConfigType::boolean},
    {0x2A0B0DF3, ConfigType::color},   {0x635F47DE, ConfigType::color},
    {0xF9200DEF, ConfigType::color},   {0x70BA27E0, ConfigType::integer},
    {0x9B4090D9, ConfigType::integer}, {0x8BED781E, ConfigType::integer},
    {0x2E5DD918, ConfigType::integer}, {0x4C044F71, ConfigType::integer},
    {0x921CAA0A, ConfigType::integer}, {0x3685FCE3, ConfigType::hotkey},
};
}

EffectsMenu::EffectsMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities,
                         effects::CameraRuntime& camera)
    : tab_(tab), category_(&tab, u"Effects", {12, 12, 12, 12}, 9, true, identities),
      camera_(camera) {
    require_settings(settings, effects_settings);
    if (&camera.force_dead != value<BooleanConfig>(settings, 0x264E3E21) ||
        &camera.alive_hotkey != value<HotkeyConfig>(settings, 0x3685FCE3))
        throw std::invalid_argument("Effects camera runtime must reference this menu's settings");
    storage_.reserve(std::size(effects_settings));
    bindings_.reserve(std::size(effects_settings));
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto color = [&](std::uint32_t hash) -> ColorPicker& {
        return add<ColorPicker>(category_, hash, value<ColorConfig>(settings, hash), identities);
    };
    auto slider = [&](std::uint32_t hash, std::u16string_view label, std::int32_t minimum,
                      std::int32_t maximum, std::uint32_t suffix) -> Slider& {
        return add<Slider>(category_, hash, label, minimum, maximum,
                           value<IntegerConfig>(settings, hash), true, suffix, 1.0f, identities);
    };
    checkbox(0x461D3721, u"Remove flashbang effects");
    checkbox(0x623BD4F2, u"Remove smoke grenades");
    checkbox(0x3359E29C, u"Remove fog");
    checkbox(0xFBD4768D, u"Remove skybox");
    auto& recoil = add<ComboBox>(category_, 0x70BA27E0, u"Visual recoil adjustment",
                                 value<IntegerConfig>(settings, 0x70BA27E0), false, identities);
    recoil.set_options({{u"Off", 0}, {u"Remove shake", 1}, {u"Remove all", 2}});
    slider(0x9B4090D9, u"Transparent walls", 1, 100, 0x25);
    slider(0x8BED781E, u"Transparent props", 1, 100, 0x25);
    auto& brightness = add<ComboBox>(category_, 0x2E5DD918, u"Brightness adjustment",
                                     value<IntegerConfig>(settings, 0x2E5DD918), false, identities);
    brightness.set_options({{u"Off", 0}, {u"Fullbright", 1}, {u"Night mode", 2}});
    brightness.callbacks.push_back({this, brightness_changed}); // 0x433EFA40
    night_color_ = &color(0x2A0B0DF3);
    night_color_->set_visible(false);
    checkbox(0x171EED00, u"Remove scope overlay");
    checkbox(0xEAFCEAD9, u"Instant scope");
    checkbox(0xA8321370, u"Disable post processing");
    checkbox(0xD35F8540, u"Force third person (alive)")
        .callbacks.push_back({this, alive_camera_changed});
    add<HotkeyWidget>(category_, 0x3685FCE3, value<HotkeyConfig>(settings, 0x3685FCE3), true,
                      identities);
    checkbox(0x264E3E21, u"Force third person (dead)")
        .callbacks.push_back({this, dead_camera_changed});
    checkbox(0x4CBB0E86, u"Disable rendering of teammates");
    checkbox(0xB589F662, u"Disable rendering of ragdolls");
    checkbox(0x3A5285A6, u"Bullet tracers");
    color(0x635F47DE);
    auto& impacts = checkbox(0xD54A1CBD, u"Bullet impacts");
    auto& duration = slider(0x4C044F71, u"", 1, 20, 0x73);
    duration.set_visible(false);
    dependencies_[0] = {&duration, &impacts.tied};
    impacts.tied.next = &dependencies_[0];
    auto& molotovs = checkbox(0xC82CBD3B, u"Modulate harmless molotovs");
    color(0xF9200DEF);
    auto& amount = slider(0x921CAA0A, u"", 0, 100, 0x25);
    amount.set_visible(false);
    dependencies_[1] = {&amount, &molotovs.tied};
    molotovs.tied.next = &dependencies_[1];
    tab.categories.push_back(&category_);
}
EffectsMenu::~EffectsMenu() {
    std::erase(tab_.categories, &category_);
}

void EffectsMenu::brightness_changed(void* context, Widget& source) { // 0x433B6FB3
    auto& combo = static_cast<ComboBox&>(source);
    static_cast<EffectsMenu*>(context)->night_color_->set_visible(combo.value && *combo.value == 2);
}
void EffectsMenu::alive_camera_changed(void* context, Widget& source) { // 0x433FFD9B / 0x4332C2FB
    auto& checkbox = static_cast<Checkbox&>(source);
    static_cast<EffectsMenu*>(context)->camera_.alive_changed(checkbox.value &&
                                                              *checkbox.value != 0);
}
void EffectsMenu::dead_camera_changed(void* context, Widget&) { // 0x434076B5 / 0x433662E6
    static_cast<EffectsMenu*>(context)->camera_.dead_changed();
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: legit_config.cpp
// ============================================================================

namespace skeet::legit {
namespace {
void bytes(ConfigLink& registry, BooleanArrayConfig& descriptor, std::uint32_t hash,
           std::uint8_t value = 0) noexcept {
    descriptor.hash = hash;
    descriptor.serialized_size = 9;
    descriptor.type = ConfigType::integer_array;
    append_config_link(registry, descriptor);
    descriptor.value.fill(value);
}
void keys(ConfigRegistries& registries, HotkeyConfigs& descriptors, std::uint32_t base) noexcept {
    for (std::uint32_t index = 0; index != descriptors.size(); ++index)
        initialize_hotkey_config(registries, descriptors[index], hotkey_hash(base, index), 6,
                                 HotkeyMode::hold);
}
}
void register_settings(ConfigRegistries& r, Settings& s) noexcept {
    auto integer = [&](IntegerArrayConfig& d, std::uint32_t h, std::int32_t v) {
        initialize_integer_array_config(r.descriptors, d, h, v);
    };
    bytes(r.descriptors, s.enabled, 0x0BB660A3);
    integer(s.maximum_fov, 0x1435B790, 100);
    integer(s.speed, 0x9D9E24B1, 65);
    integer(s.attack_speed, 0xC68AC43E, 65);
    integer(s.speed_scale_fov, 0xF7648818, 100);
    integer(s.reaction_time, 0x836DBF4B, 100);
    bytes(r.descriptors, s.through_smoke, 0xE7B78410);
    bytes(r.descriptors, s.while_blind, 0xAFD7D6D3);
    bytes(r.descriptors, s.quick_stop, 0xE9197E9C);
    integer(s.recoil_pitch, 0xA2FBFAC6, 100);
    integer(s.recoil_yaw, 0x1716EAD7, 75);
    integer(s.maximum_lock_time, 0x8946733C, 1001);
    bytes(r.descriptors, s.head, 0xB9327935, 1);
    bytes(r.descriptors, s.chest, 0x6ECF9C72, 1);
    bytes(r.descriptors, s.stomach, 0x771C5252, 1);
    integer(s.first_shot_delay, 0xA09C5083, 0);
    bytes(r.descriptors, s.trigger_enabled, 0xEE182BDA);
    integer(s.minimum_hit_chance, 0x5BDB158D, 75);
    integer(s.minimum_damage, 0x47A24717, 1);
    bytes(r.descriptors, s.burst_fire, 0xD1DC4D24);
    integer(s.burst_time, 0x5BC951E6, 250);
    bytes(r.descriptors, s.config_8a823c30, 0x8A823C30);
    integer(s.config_90b2820a, 0x90B2820A, 200);
    bytes(r.descriptors, s.penetration, 0x6D349307);
    bytes(r.descriptors, s.shoot_smoke, 0xB870359F);
    bytes(r.descriptors, s.shoot_blind, 0x6FDB5A10);
    bytes(r.descriptors, s.trigger_head, 0xBE6224C0, 1);
    bytes(r.descriptors, s.trigger_chest, 0xC8E339F1, 1);
    bytes(r.descriptors, s.trigger_stomach, 0xCFCEE7E5, 1);
    integer(s.trigger_reaction, 0x6CEA053A, 0);
    integer(s.accuracy_boost, 0xFDE7159F, 0);
    integer(s.accuracy_range, 0x2D41A289, 32);
    bytes(r.descriptors, s.standalone_recoil, 0x2BE03F3A);
    keys(r, s.aim_keys, 0xA31AE814);
    keys(r, s.trigger_keys, 0xA53DC8B7);
}
void finish_settings_initialization(Settings& s) noexcept {
    s.head.value[4] = s.head.value[5] = 0;
    s.recoil_pitch.value[5] = s.recoil_yaw.value[5] = 0;
    s.chest.value[5] = s.stomach.value[5] = 1;
}
void initialize_settings(ConfigRegistries& r, Settings& s) noexcept {
    register_settings(r, s);
    finish_settings_initialization(s);
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: legit_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
struct ArrayRequirement {
    std::uint32_t hash;
    std::uint16_t size;
};
constexpr ArrayRequirement arrays[] = {
    {0x0BB660A3, 9},  {0x1435B790, 36}, {0x9D9E24B1, 36}, {0xC68AC43E, 36}, {0xF7648818, 36},
    {0x836DBF4B, 36}, {0xE7B78410, 9},  {0xAFD7D6D3, 9},  {0xE9197E9C, 9},  {0xA2FBFAC6, 36},
    {0x1716EAD7, 36}, {0x8946733C, 36}, {0xB9327935, 9},  {0x6ECF9C72, 9},  {0x771C5252, 9},
    {0xA09C5083, 36}, {0xEE182BDA, 9},  {0x5BDB158D, 36}, {0x47A24717, 36}, {0xD1DC4D24, 9},
    {0x5BC951E6, 36}, {0x8A823C30, 9},  {0x90B2820A, 36}, {0x6D349307, 9},  {0xB870359F, 9},
    {0x6FDB5A10, 9},  {0xBE6224C0, 9},  {0xC8E339F1, 9},  {0xCFCEE7E5, 9},  {0x6CEA053A, 36},
    {0xFDE7159F, 36}, {0x2D41A289, 36}, {0x2BE03F3A, 9},
};
}

LegitMenu::LegitMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), weapon_(&tab, u"Weapon type", {0, 24, 0, -1}, 0, true, identities) {
    tab.categories.push_back(&weapon_);
    for (const auto& legit_required : arrays) {
        auto& d = descriptor(settings, legit_required.hash);
        if (d.type != ConfigType::integer_array || d.serialized_size != legit_required.size)
            throw std::invalid_argument("LEGIT requires the native array descriptor size and type");
    }
    for (std::uint32_t i = 0; i != 9; ++i) {
        auto& aim = descriptor(settings, legit::hotkey_hash(0xA31AE814, i));
        auto& trigger = descriptor(settings, legit::hotkey_hash(0xA53DC8B7, i));
        if (aim.type != ConfigType::hotkey || trigger.type != ConfigType::hotkey ||
            aim.serialized_size != 8 || trigger.serialized_size != 8)
            throw std::invalid_argument("LEGIT requires its eighteen native hotkey descriptors");
        aim_keys_[i] = &static_cast<HotkeyConfig&>(aim);
        trigger_keys_[i] = &static_cast<HotkeyConfig&>(trigger);
    }
    storage_.reserve(35);
    bindings_.reserve(35);
    arrays_.reserve(31);
    weapon_.default_fixed_height = weapon_.fixed_height = 60;
    weapon_.resizable = weapon_.movable = false;
    selector_ = &add<LegitWeaponSelector>(weapon_, 0, &selected_weapon_, identities);
    selector_->callbacks.push_back({this, weapon_changed}); // 0x433E0A18
    auto checkbox = [&](Category& owner, std::uint32_t hash,
                        std::u16string_view label) -> Checkbox& {
        auto& d = descriptor(settings, hash);
        auto& w = add<Checkbox>(owner, hash, label,
                                &static_cast<legit::BooleanArrayConfig&>(d).value[1], identities);
        arrays_.push_back({&w, &d, true});
        return w;
    };
    auto slider = [&](Category& owner, std::uint32_t hash, std::u16string_view label,
                      std::int32_t minimum, std::int32_t maximum, std::uint32_t suffix = 0,
                      float scale = 1.0f) -> Slider& {
        auto& d = descriptor(settings, hash);
        auto& w = add<Slider>(owner, hash, label, minimum, maximum,
                              &static_cast<IntegerArrayConfig&>(d).value[1], true, suffix, scale,
                              identities);
        arrays_.push_back({&w, &d, false});
        return w;
    };
    aimbot_ = std::make_unique<Category>(&tab, u"Aimbot", GridPosition{0, 12, 0, 24}, 14, true,
                                         identities);
    tab.categories.push_back(aimbot_.get());
    checkbox(*aimbot_, 0x0BB660A3, u"Enabled");
    aim_key_ = &add<HotkeyWidget>(*aimbot_, 0xA31AE814, &aim_keys_[1]->value, true, identities);
    aim_key_->callbacks.push_back({this, aim_key_changed}); // 0x4336ED87
    slider(*aimbot_, 0x9D9E24B1, u"Speed", 0, 700, 0, 0.01f);
    slider(*aimbot_, 0xC68AC43E, u"Speed (in attack)", 0, 700, 0, 0.01f);
    slider(*aimbot_, 0xF7648818, u"Speed scale - FOV", 1, 100, 0x25);
    auto& lock = slider(*aimbot_, 0x8946733C, u"Maximum lock-on time", 1, 1001, 0x73006D);
    lock.value_labels.emplace(1001, L"\u221E");
    lock.update_value_text();
    slider(*aimbot_, 0x836DBF4B, u"Reaction time", 0, 200, 0x73006D);
    slider(*aimbot_, 0x1435B790, u"Maximum FOV", 1, 900, 0xB0, 0.1f);
    add<Label>(*aimbot_, 0, u"Recoil compensation (P/Y)", identities);
    slider(*aimbot_, 0xA2FBFAC6, u"", 0, 125, 0x25);
    slider(*aimbot_, 0x1716EAD7, u"", 0, 100, 0x25);
    checkbox(*aimbot_, 0xE9197E9C, u"Quick stop");
    checkbox(*aimbot_, 0xE7B78410, u"Aim through smoke");
    checkbox(*aimbot_, 0xAFD7D6D3, u"Aim while blind");
    checkbox(*aimbot_, 0xB9327935, u"Head");
    checkbox(*aimbot_, 0x6ECF9C72, u"Chest");
    checkbox(*aimbot_, 0x771C5252, u"Stomach");
    first_delay_ = &slider(*aimbot_, 0xA09C5083, u"First shot delay range", 0, 64, 0x6E0069);
    first_delay_->set_visible(false);
    first_delay_->value_labels.emplace(0, L"Off");
    first_delay_->update_value_text();

    trigger_ = std::make_unique<Category>(&tab, u"Triggerbot", GridPosition{12, 12, 0, 16}, 12,
                                          true, identities);
    tab.categories.push_back(trigger_.get());
    checkbox(*trigger_, 0xEE182BDA, u"Enabled");
    trigger_key_ =
        &add<HotkeyWidget>(*trigger_, 0xA53DC8B7, &trigger_keys_[1]->value, true, identities);
    slider(*trigger_, 0x5BDB158D, u"Minimum hit chance", 0, 100, 0x25);
    trigger_reaction_ = &slider(*trigger_, 0x6CEA053A, u"Reaction time", 0, 200, 0x73006D);
    trigger_reaction_->callbacks.push_back({this, reaction_changed}); // 0x4336B6EB
    burst_ = &checkbox(*trigger_, 0xD1DC4D24, u"Burst fire");
    burst_time_ = &slider(*trigger_, 0x5BC951E6, u"", 1, 500, 0x73006D);
    burst_time_->set_visible(false);
    burst_link_ = {burst_time_, &burst_->tied};
    burst_->tied.next = &burst_link_;
    slider(*trigger_, 0x47A24717, u"Minimum damage", 1, 100);
    penetration_ = &checkbox(*trigger_, 0x6D349307, u"Automatic penetration");
    checkbox(*trigger_, 0xB870359F, u"Shoot through smoke");
    checkbox(*trigger_, 0x6FDB5A10, u"Shoot while blind");
    checkbox(*trigger_, 0xBE6224C0, u"Head");
    checkbox(*trigger_, 0xC8E339F1, u"Chest");
    checkbox(*trigger_, 0xCFCEE7E5, u"Stomach");

    other_ = std::make_unique<Category>(&tab, u"Other", GridPosition{12, 12, 16, 8}, 0, true,
                                        identities);
    tab.categories.push_back(other_.get());
    auto& accuracy_d = descriptor(settings, 0xFDE7159F);
    accuracy_ =
        &add<ComboBox>(*other_, 0xFDE7159F, u"Accuracy boost",
                       &static_cast<IntegerArrayConfig&>(accuracy_d).value[1], false, identities);
    accuracy_->set_options({{u"Off", 0}, {u"Low", 1}, {u"High", 2}});
    arrays_.push_back({accuracy_, &accuracy_d, false});
    accuracy_->callbacks.push_back({this, accuracy_changed}); // 0x433B7738
    accuracy_range_ = &slider(*other_, 0x2D41A289, u"Accuracy boost range", 1, 64, 0x6E0069);
    accuracy_range_->set_visible(false);
    accuracy_range_->value_labels.emplace(64, L"\u221E");
    accuracy_range_->update_value_text();
    checkbox(*other_, 0x2BE03F3A, u"Standalone recoil compensation");
    for (auto* category : categories())
        category->reflow(); // 0x433CF58B
}
LegitMenu::~LegitMenu() {
    for (auto* category : categories())
        std::erase(tab_.categories, category);
}
void LegitMenu::weapon_changed(void* context, Widget&) {
    static_cast<LegitMenu*>(context)->bind_weapon(); // 0x4334B817
}
void LegitMenu::aim_key_changed(void* context, Widget&) {
    static_cast<LegitMenu*>(context)->first_delay_->set_visible(false); // 0x4336ED87
}
void LegitMenu::reaction_changed(void* context, Widget&) { // 0x4341F19E
    auto& self = *static_cast<LegitMenu*>(context);
    const auto delay = self.trigger_reaction_->value ? *self.trigger_reaction_->value : 0;
    if (delay != 0)
        self.penetration_->set_value(false);
    self.penetration_->enabled = delay == 0;
}
void LegitMenu::accuracy_changed(void* context, Widget&) { // 0x4332927A
    auto& self = *static_cast<LegitMenu*>(context);
    self.accuracy_range_->set_visible(self.accuracy_->value && *self.accuracy_->value != 0);
}
void LegitMenu::bind_weapon() { // 0x4334B817..0x4334BB75
    if (selected_weapon_ < 0 || selected_weapon_ >= 9)
        return;
    const auto index = std::size_t(selected_weapon_);
    aim_key_->value = &aim_keys_[index]->value;
    aim_key_->config_loaded();
    for (std::size_t i = 0; i != arrays_.size(); ++i) {
        if (i == 16) {
            trigger_key_->value = &trigger_keys_[index]->value;
            trigger_key_->config_loaded();
        }
        auto& binding = arrays_[i];
        if (binding.bytes)
            static_cast<Checkbox*>(binding.widget)->value =
                &static_cast<legit::BooleanArrayConfig&>(*binding.descriptor).value[index];
        else {
            auto* value = &static_cast<IntegerArrayConfig&>(*binding.descriptor).value[index];
            if (auto* slider = dynamic_cast<Slider*>(binding.widget))
                slider->value = value;
            else
                static_cast<ComboBox*>(binding.widget)->value =
                    reinterpret_cast<std::uint32_t*>(value);
        }
        binding.widget->config_loaded();
    }
    reaction_changed(this, *trigger_reaction_);
    first_delay_->set_visible(false);
    accuracy_changed(this, *accuracy_);
    burst_time_->set_visible(burst_->value && *burst_->value != 0);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: legit_weapon_selector.cpp
// ============================================================================

namespace skeet::ui {
namespace {
using i32 = std::int32_t;
i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
i32 mul(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * std::uint32_t(b));
}
}
LegitWeaponSelector::LegitWeaponSelector(Widget* owner, std::int32_t* selection,
                                         WidgetIdentities& identities) // 0x43484CFD
    : ComboBox(owner, u"Weapon type", selection, true, identities) {
    open = true;
    geometry.horizontal_offset = 0;
    geometry.bounds.x = 10;
    geometry.bounds.y = 0;
    geometry.bounds.height = geometry.default_size.y = 60;
    set_options({{u"Pistol", 1},
                 {u"SMG", 2},
                 {u"Rifle", 3},
                 {u"Shotgun", 4},
                 {u"Machine gun", 6},
                 {u"Sniper", 5}});
}
void LegitWeaponSelector::reset_input() { // 0x433F4BA9
    hovered_item = -1;
    hovered = held = false;
    open = true;
    scroll.dragging = false;
}
bool LegitWeaponSelector::hit_test(Point cursor) const { // 0x4334A278
    auto* category = owning_category(const_cast<LegitWeaponSelector*>(this));
    if (!category)
        throw std::logic_error("Weapon selector requires its native owning category");
    return geometry.bounds.x <= cursor.x &&
           cursor.x <= add(category->geometry.bounds.width, geometry.bounds.x) &&
           geometry.bounds.y <= cursor.y &&
           cursor.y <= add(category->geometry.bounds.height, geometry.bounds.y);
}
void LegitWeaponSelector::interact_weapons(const InputMessage& message,
                                           Point cursor) { // 0x4339DF7F
    hovered_item = -1;
    const auto relative_x = sub(cursor.x, geometry.bounds.x);
    if (sub(cursor.y, geometry.bounds.y) < 0 || relative_x < 0)
        return;
    if (cell_width == 0 || options.empty())
        throw std::logic_error("Weapon selector requires native nonempty options and cell width");
    const auto index =
        std::min(std::uint32_t(relative_x / cell_width), std::uint32_t(options.size() - 1));
    hovered_item = std::int32_t(index);
    if (held && (value ? *value : 0) != options[index].value)
        set_value(options[index].value);
    if (message.message == 0x202)
        open = held = false;
}
bool LegitWeaponSelector::mouse(const InputMessage& message,
                                Point cursor) { // 0x433E8C81 -> 0x4340C49A
    const auto result = [&]() {
        if (!enabled)
            return false;
        if (open && scroll.enabled &&
            scrollbar_mouse(scroll, message.message,
                            {sub(cursor.x, geometry.bounds.x),
                             sub(sub(cursor.y, row_height), geometry.bounds.y)},
                            scale))
            return true;
        if (message.message == 0x200) {
            if (!hit_test(cursor)) {
                hovered_item = -1;
                hovered = held = false;
                return false;
            }
            hovered = true;
            if (open)
                interact_weapons(message, cursor);
            return held;
        }
        if (message.message == 0x201) {
            if (!hit_test(cursor))
                return false;
            held = true;
            return true;
        }
        if (message.message == 0x202) {
            scroll.dragging = false;
            if (!held)
                return false;
            if (hit_test(cursor)) {
                if (!open) {
                    open = true;
                    held = false;
                } else
                    interact_weapons(message, cursor);
                return true;
            }
            held = false;
            return false;
        }
        if (message.message == 0x204 || message.message == 0x205)
            return hit_test(cursor);
        if (message.message == 0x20A && open && scroll.enabled && hit_test(cursor)) {
            const auto wheel = std::bit_cast<std::int16_t>(std::uint16_t(message.parameter >> 16));
            move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height,
                                 row_height * (wheel > 0 ? -4 : 4));
            return true;
        }
        return false;
    }();
    open = true;
    return result;
}
void LegitWeaponSelector::resize(const SliderMetrics&) { // 0x433B2F44
    auto* category = owning_category(this);
    if (!category)
        throw std::logic_error("Weapon selector resize requires its owning category");
    geometry.bounds.width = category->geometry.bounds.width;
    geometry.bounds.height = category->geometry.bounds.height;
}
void LegitWeaponSelector::dpi_changed(float factor, const SliderMetrics& metrics) { // 0x4336E814
    scale = factor;
    resize(metrics);
    cell_width = round_layout_coordinate(76.0f * factor);
    icon_height = round_layout_coordinate(20.0f * factor);
}
void LegitWeaponSelector::on_layout() { // 0x4338AAFF
    geometry.bounds.x = add(round_layout_coordinate(scale * 10.0f), geometry.horizontal_offset);
    geometry.bounds.y = 0;
}
void LegitWeaponSelector::draw(RenderQueue& queue, Point origin,
                               std::uint32_t) const { // 0x4336F5CA
    if (!icons.load)
        throw std::logic_error("Weapon selector drawing requires its native icon cache provider");
    constexpr std::uint16_t definitions[]{61, 19, 7, 27, 28, 40};
    auto x = add(origin.x, geometry.bounds.x);
    const auto y = add(origin.y, geometry.bounds.y);
    for (std::size_t i = 0; i != options.size(); ++i) {
        esp::WeaponIcon icon;
        if (i >= std::size(definitions))
            throw std::logic_error("Weapon selector options exceed the native icon table");
        if (icons.load(icons.context, queue, definitions[i], definitions[i], icon_height, icon)) {
            const auto color = i == selected_item                 ? 0xC8FFFFFFu
                               : i == std::uint32_t(hovered_item) ? 0x78FFFFFFu
                                                                  : 0x50FFFFFFu;
            auto icon_x = x;
            if (icon.width < cell_width)
                icon_x = add(icon_x, sub(cell_width, icon.width) / 2);
            auto icon_y = y;
            if (icon.height < geometry.bounds.height)
                icon_y = add(icon_y, sub(geometry.bounds.height, icon.height) / 2);
            queue.texture({icon_x, icon_y, std::uint32_t(icon.texture), color, 0, 0, icon.width,
                           icon.height, 0, 1.0f});
        }
        x = add(x, cell_width);
    }
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: misc_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement misc_required[] = {
    {0xFC225A4F, ConfigType::integer}, {0xCA80F9ED, ConfigType::integer},
    {0x7A5909EE, ConfigType::boolean}, {0xA7C81D9B, ConfigType::integer},
    {0x2E314CE2, ConfigType::boolean}, {0x3D5DE026, ConfigType::boolean},
    {0x3952AAA6, ConfigType::integer}, {0x0D634908, ConfigType::boolean},
    {0x30E0CFE7, ConfigType::boolean}, {0xD351D519, ConfigType::boolean},
    {0x141F80B7, ConfigType::boolean}, {0x9F6C4676, ConfigType::boolean},
    {0x5086BCF5, ConfigType::boolean}, {0x0CE14F5A, ConfigType::boolean},
    {0x969EEF1F, ConfigType::boolean}, {0xDB791DB8, ConfigType::hotkey},
    {0x510F682D, ConfigType::integer}, {0x37A61386, ConfigType::boolean},
    {0x916F47DA, ConfigType::boolean}, {0x3C4135FF, ConfigType::hotkey},
    {0xC5290E81, ConfigType::integer}, {0x77D31850, ConfigType::hotkey},
    {0x245CEDB5, ConfigType::boolean}, {0x473CC2DE, ConfigType::hotkey},
    {0x177AB88C, ConfigType::boolean}, {0x6ED248E1, ConfigType::boolean},
    {0x52EADF91, ConfigType::boolean},
};
}

MiscellaneousMenu::MiscellaneousMenu(Tab& tab, ConfigRegistries& settings,
                                     WidgetIdentities& identities, misc::Runtime& runtime,
                                     std::uint8_t& upgrade_tablet, bool expose_tablet)
    : tab_(tab), category_(&tab, u"Miscellaneous", {0, 12, 0, 24}, 13, true, identities),
      runtime_(runtime) {
    require_settings(settings, misc_required);
    runtime_.auto_accept_enabled = value<BooleanConfig>(settings, 0x141F80B7);
    runtime_.upgrade_tablet_enabled = &upgrade_tablet;
    runtime_.clan_tag.enabled = value<BooleanConfig>(settings, 0x9F6C4676);
    runtime_.ping_spike.enabled = value<BooleanConfig>(settings, 0x916F47DA);
    runtime_.ping_spike.hotkey = value<HotkeyConfig>(settings, 0x3C4135FF);
    runtime_.ping_spike.milliseconds = value<IntegerConfig>(settings, 0xC5290E81);
    if (runtime_.rebuy_fix)
        runtime_.rebuy_fix->enabled = value<BooleanConfig>(settings, 0x6ED248E1);
    if (runtime_.quick_switch)
        runtime_.quick_switch->enabled = value<BooleanConfig>(settings, 0x0D634908);
    if (runtime_.reveal_ranks)
        runtime_.reveal_ranks->enabled = value<BooleanConfig>(settings, 0x30E0CFE7);
    storage_.reserve(32);
    bindings_.reserve(32);
    tab.categories.push_back(&category_);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view name) -> Checkbox& {
        return add<Checkbox>(category_, hash, name, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto slider = [&](std::uint32_t hash, std::u16string_view name, int minimum, int maximum,
                      std::uint32_t suffix) -> Slider& {
        return add<Slider>(category_, hash, name, minimum, maximum,
                           value<IntegerConfig>(settings, hash), true, suffix, 1.0f, identities);
    };
    auto key = [&](std::uint32_t hash) -> HotkeyWidget& {
        return add<HotkeyWidget>(category_, hash, value<HotkeyConfig>(settings, hash), true,
                                 identities);
    };
    auto tie = [&](std::size_t index, Checkbox& source, Widget& target) {
        target.set_visible(false);
        dependencies_[index] = {&target, &source.tied};
        source.tied.next = &dependencies_[index];
    };
    slider(0xFC225A4F, u"Override FOV", 1, 135, 0xB0);
    slider(0xCA80F9ED, u"Override zoom FOV", 0, 100, 0x25);
    auto& knife = checkbox(0x7A5909EE, u"Knifebot");
    auto& knife_mode = add<MultiSelect>(category_, 0xA7C81D9B, u"",
                                        value<IntegerConfig>(settings, 0xA7C81D9B), identities);
    knife_mode.add_option("Swing", 1);
    knife_mode.add_option("Full stab", 2);
    tie(0, knife, knife_mode);
    checkbox(0x2E314CE2, u"Zeusbot");
    auto& automatic = checkbox(0x3D5DE026, u"Automatic weapons");
    tie(1, automatic, slider(0x3952AAA6, u"", 0, 200, 0x0073006D));
    checkbox(0x0D634908, u"Quick switch");
    checkbox(0x30E0CFE7, u"Reveal competitive ranks");
    checkbox(0xD351D519, u"Reveal Overwatch players");
    checkbox(0x141F80B7, u"Auto-accept matchmaking")
        .callbacks.push_back({this, auto_accept_changed});
    checkbox(0x9F6C4676, u"Clan tag spammer");
    checkbox(0x5086BCF5, u"Log weapon purchases");
    checkbox(0x0CE14F5A, u"Log damage dealt");
    auto& grenade = checkbox(0x969EEF1F, u"Automatic grenade release");
    key(0xDB791DB8);
    tie(2, grenade, slider(0x510F682D, u"Minimum grenade damage", 1, 50, 0));
    checkbox(0x37A61386, u"Super toss");
    auto& ping = checkbox(0x916F47DA, u"Ping spike");
    ping.callbacks.push_back({this, ping_spike_changed});
    key(0x3C4135FF);
    tie(3, ping, slider(0xC5290E81, u"", 1, 200, 0x0073006D));
    add<Label>(category_, 0, u"Free look", identities);
    key(0x77D31850);
    checkbox(0x245CEDB5, u"Persistent kill feed");
    add<Label>(category_, 0, u"Last second defuse", identities);
    key(0x473CC2DE);
    checkbox(0x177AB88C, u"Disable sv_pure");
    if (expose_tablet)
        add<Checkbox>(category_, 0, u"Upgrade tablet", &upgrade_tablet, identities)
            .callbacks.push_back({this, upgrade_tablet_changed});
    checkbox(0x6ED248E1, u"Rebuy fix");
    checkbox(0x52EADF91, u"Draw console output");
    add<Button>(category_, 0, u"Steal player name", Point{158, 28},
                WidgetCallback{this, steal_player_name}, identities);
    add<Button>(category_, 0, u"Dump MM wins", Point{158, 28},
                WidgetCallback{this, dump_matchmaking_wins}, identities);
}
MiscellaneousMenu::~MiscellaneousMenu() {
    std::erase(tab_.categories, &category_);
}
void MiscellaneousMenu::auto_accept_changed(void* context, Widget&) {
    static_cast<MiscellaneousMenu*>(context)->runtime_.auto_accept_changed();
}
void MiscellaneousMenu::ping_spike_changed(void* context, Widget& widget) {
    const auto* setting = static_cast<Checkbox&>(widget).value;
    static_cast<MiscellaneousMenu*>(context)->runtime_.ping_spike_changed(setting && *setting != 0);
}
void MiscellaneousMenu::upgrade_tablet_changed(void* context, Widget&) {
    static_cast<MiscellaneousMenu*>(context)->runtime_.upgrade_tablet_changed();
}
void MiscellaneousMenu::steal_player_name(void* context, Widget&) {
    static_cast<MiscellaneousMenu*>(context)->runtime_.steal_player_name();
}
void MiscellaneousMenu::dump_matchmaking_wins(void* context, Widget&) {
    static_cast<MiscellaneousMenu*>(context)->runtime_.dump_matchmaking_wins();
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: misc_settings_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement misc_settings_required[] = {
    {0x851E6ABE, ConfigType::boolean},
    {0x526A2BF4, ConfigType::boolean},
    {0x643364E6, ConfigType::boolean},
    {0x740C32AE, ConfigType::boolean},
};
}
SettingsMenu::SettingsMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities,
                           SettingsMenuValues values, misc::SettingsRuntime& runtime)
    : tab_(tab), category_(&tab, u"Settings", {12, 12, 12, 12}, 8, true, identities),
      runtime_(runtime) {
    require_settings(settings, misc_settings_required);
    runtime_.dpi_scale = &values.dpi_scale;
    runtime_.lock_layout = &values.lock_layout;
    runtime_.allow_custom_events = value<BooleanConfig>(settings, 0x526A2BF4);
    storage_.reserve(21);
    bindings_.reserve(21);
    tab.categories.push_back(&category_);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view name) -> Checkbox& {
        return add<Checkbox>(category_, hash, name, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto direct_checkbox = [&](std::u16string_view name, std::uint8_t& setting) -> Checkbox& {
        return add<Checkbox>(category_, 0, name, &setting, identities);
    };
    auto slider = [&](std::u16string_view name, int low, int high, std::int32_t& setting,
                      std::uint32_t suffix) -> Slider& {
        return add<Slider>(category_, 0, name, low, high, &setting, true, suffix, 1.0f, identities);
    };
    std::array<Widget*, 10> hidden{};
    std::size_t hidden_count = 0;
    auto hide = [&](Widget& control) -> Widget& {
        hidden[hidden_count++] = &control;
        return control;
    };

    add<Label>(category_, 0, u"Menu key", identities);
    add<HotkeyWidget>(category_, 0, &values.menu_key, false, identities);
    add<Label>(category_, 0, u"Menu color", identities);
    add<ColorPicker>(category_, 0, &values.menu_color, identities);
    auto& dpi = add<ComboBox>(category_, 0, u"DPI scale", &values.dpi_scale, false, identities);
    if (runtime_.gui_dpi)
        runtime_.gui_dpi->setting_widget = &dpi;
    dpi.set_options({{u"100%", 0}, {u"125%", 1}, {u"150%", 2}, {u"175%", 3}, {u"200%", 4}});
    dpi.callbacks.push_back({this, dpi_scale_changed});
    checkbox(0x851E6ABE, u"Anti-untrusted");
    auto& custom = checkbox(0x526A2BF4, u"Allow custom game events");
    hide(custom);
    custom.callbacks.push_back({this, allow_custom_events_changed});
    custom.color = 0xBB82F0F0;
    checkbox(0x643364E6, u"Hide from OBS");
    checkbox(0x740C32AE, u"Low FPS warning");
    direct_checkbox(u"Lock menu layout", values.lock_layout)
        .callbacks.push_back({this, lock_layout_changed});
    hide(direct_checkbox(u"Multithreading", values.multithreading));
    add<Button>(category_, 0, u"Reset menu layout", Point{158, 28},
                WidgetCallback{this, reset_layout}, identities);
    add<Button>(category_, 0, u"Unload", Point{158, 28}, WidgetCallback{this, unload}, identities);
    auto& unlag = slider(u"sv_maxunlag2", 200, 1000, values.maximum_unlag, 0x0073006D);
    hide(unlag);
    unlag.callbacks.push_back({this, maximum_unlag_changed});
    auto& ticks = slider(u"sv_maxusrcmdprocessticks2", 4, 62, values.maximum_usercmd_ticks, 0);
    maximum_usercmd_ticks_ = &ticks;
    hide(ticks);
    ticks.callbacks.push_back({this, maximum_usercmd_ticks_changed});
    hide(slider(u"sv_clockcorrection_msecs2", 0, 1000, values.clock_correction, 0x0073006D));
    hide(direct_checkbox(u"sv_maxusrcmdprocessticks_holdaim", values.hold_aim));
    hide(direct_checkbox(u"Fix viewmodel lag", values.fix_viewmodel_lag));
    hide(direct_checkbox(u"Fix networking", values.fix_networking));
    hide(direct_checkbox(u"Fix animations", values.fix_animations));
    hide(direct_checkbox(u"Faster grenade toss", values.faster_grenade_toss));
    for (auto* control : hidden)
        control->set_visible(false);
}
SettingsMenu::~SettingsMenu() {
    std::erase(tab_.categories, &category_);
}
void SettingsMenu::dpi_scale_changed(void* context, Widget& widget) {
    widget.reset_input(); // source vfunc +0x18 in 0x433379D4
    auto& runtime = static_cast<SettingsMenu*>(context)->runtime_;
    runtime.dpi_scale_changed();
}
void SettingsMenu::allow_custom_events_changed(void* context, Widget&) {
    static_cast<SettingsMenu*>(context)->runtime_.allow_custom_events_changed();
}
void SettingsMenu::lock_layout_changed(void* context, Widget&) {
    static_cast<SettingsMenu*>(context)->runtime_.lock_layout_changed();
}
void SettingsMenu::reset_layout(void* context, Widget&) {
    static_cast<SettingsMenu*>(context)->runtime_.reset_menu_layout();
}
void SettingsMenu::unload(void* context, Widget&) {
    static_cast<SettingsMenu*>(context)->runtime_.request_unload();
}
void SettingsMenu::maximum_unlag_changed(void*, Widget&) {
    // 0x433CCE8E is a native return instruction.
}
void SettingsMenu::maximum_usercmd_ticks_changed(void* context, Widget& widget) {
    auto* setting = static_cast<Slider&>(widget).value;
    static_cast<SettingsMenu*>(context)->runtime_.max_usercmd_ticks_changed(*setting);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: movement_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement movement_settings[] = {
    {0x4CD278CA, ConfigType::boolean}, {0xD0CA5B87, ConfigType::boolean},
    {0xDB2172A1, ConfigType::boolean}, {0xFE3FDAAB, ConfigType::boolean},
    {0xF2EB89E9, ConfigType::boolean}, {0x6F6ACC9B, ConfigType::integer},
    {0xC94C5F0B, ConfigType::integer}, {0x3095C574, ConfigType::boolean},
    {0x3FDF2566, ConfigType::boolean}, {0x2B499E77, ConfigType::hotkey},
    {0x3027DC11, ConfigType::integer}, {0x3127DE82, ConfigType::integer},
    {0xCA7FC107, ConfigType::boolean}, {0x712C1936, ConfigType::hotkey},
    {0x8E38F1F3, ConfigType::boolean}, {0x6E4E5CC7, ConfigType::color},
    {0x5B1A5C7F, ConfigType::integer}, {0xB2CDBB7D, ConfigType::boolean},
    {0xE2FFCCF0, ConfigType::hotkey},  {0x8C13C109, ConfigType::boolean},
    {0x5BBB224E, ConfigType::hotkey},  {0xB5E64AD1, ConfigType::boolean},
};
}

MovementMenu::MovementMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Movement", {12, 12, 0, 12}, 13, true, identities) {
    require_settings(settings, movement_settings);
    storage_.reserve(std::size(movement_settings));
    bindings_.reserve(std::size(movement_settings));
    tab.categories.push_back(&category_);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto hotkey = [&](std::uint32_t hash) -> HotkeyWidget& {
        return add<HotkeyWidget>(category_, hash, value<HotkeyConfig>(settings, hash), true,
                                 identities);
    };
    checkbox(0x4CD278CA, u"Standalone quick stop");
    checkbox(0xD0CA5B87, u"Infinite duck").color = 0xBB82F0F0;
    checkbox(0xDB2172A1, u"Easy strafe");
    checkbox(0xFE3FDAAB, u"Bunny hop");
    auto& air_strafe = checkbox(0xF2EB89E9, u"Air strafe");
    direction_ = &add<MultiSelect>(category_, 0x6F6ACC9B, u"Air strafe direction",
                                   value<IntegerConfig>(settings, 0x6F6ACC9B), identities);
    direction_->add_option("View angles", 1);
    direction_->add_option("Movement keys", 2);
    direction_->set_visible(false);
    smoothing_ =
        &add<Slider>(category_, 0xC94C5F0B, u"Air strafe smoothing", 0, 100,
                     value<IntegerConfig>(settings, 0xC94C5F0B), true, 0x25u, 1.0f, identities);
    smoothing_->set_visible(false);
    avoid_collisions_ = &checkbox(0x3095C574, u"Avoid collisions");
    avoid_collisions_->set_visible(false);
    z_hop_ = &checkbox(0x3FDF2566, u"Z-Hop");
    z_hop_->set_visible(false);
    z_hop_key_ = &hotkey(0x2B499E77);
    z_hop_key_->set_visible(false);
    z_hop_first_ =
        &add<Slider>(category_, 0x3027DC11, u"", 100, 300,
                     value<IntegerConfig>(settings, 0x3027DC11), true, 0u, 0.1f, identities);
    z_hop_first_->set_visible(false);
    z_hop_second_ =
        &add<Slider>(category_, 0x3127DE82, u"", 100, 500,
                     value<IntegerConfig>(settings, 0x3127DE82), true, 0u, 0.1f, identities);
    z_hop_second_->set_visible(false);
    pre_speed_ = &checkbox(0xCA7FC107, u"Pre-speed");
    pre_speed_->set_visible(false);
    pre_speed_key_ = &hotkey(0x712C1936);
    pre_speed_key_->set_visible(false);
    checkbox(0x8E38F1F3, u"No fall damage");
    add<ColorPicker>(category_, 0x6E4E5CC7, value<ColorConfig>(settings, 0x6E4E5CC7), identities);
    auto& air_duck = add<ComboBox>(category_, 0x5B1A5C7F, u"Air duck",
                                   value<IntegerConfig>(settings, 0x5B1A5C7F), false, identities);
    air_duck.set_options({{u"Off", 0}, {u"On", 1}, {u"Spam", 3}, {u"Fake", 2}, {u"On land", 4}});
    checkbox(0xB2CDBB7D, u"Blockbot");
    hotkey(0xE2FFCCF0);
    checkbox(0x8C13C109, u"Jump at edge");
    hotkey(0x5BBB224E);
    checkbox(0xB5E64AD1, u"Fast walk");
    air_strafe.callbacks.push_back({this, air_strafe_changed}); // 0x4336E86C
    z_hop_->callbacks.push_back({this, z_hop_changed}); // 0x433AFC9F
}
MovementMenu::~MovementMenu() {
    std::erase(tab_.categories, &category_);
}

void MovementMenu::update_z_hop(const Checkbox& source) { // 0x43337663
    const auto visible = source.visible && source.value && *source.value != 0;
    z_hop_first_->set_visible(visible);
    z_hop_second_->set_visible(visible);
}
void MovementMenu::z_hop_changed(void* context, Widget& source) { // 0x433AFC9F
    static_cast<MovementMenu*>(context)->update_z_hop(static_cast<Checkbox&>(source));
}
void MovementMenu::air_strafe_changed(void* context, Widget& source) { // 0x433D1146
    auto& self = *static_cast<MovementMenu*>(context);
    auto& checkbox = static_cast<Checkbox&>(source);
    const auto visible = checkbox.value && *checkbox.value != 0;
    self.direction_->set_visible(visible);
    self.smoothing_->set_visible(visible);
    self.z_hop_->set_visible(visible);
    self.z_hop_key_->set_visible(visible);
    self.pre_speed_->set_visible(visible);
    self.pre_speed_key_->set_visible(visible);
    self.update_z_hop(*self.z_hop_);
    self.avoid_collisions_->set_visible(visible);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: other_esp_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement other_esp_settings[] = {
    {0xD7CD1B0B, ConfigType::boolean}, {0x25B4B670, ConfigType::boolean},
    {0xEDF6C582, ConfigType::boolean}, {0x78A1B150, ConfigType::boolean},
    {0xAD37A2EB, ConfigType::boolean}, {0xF2937E07, ConfigType::boolean},
    {0xDC4C1BB9, ConfigType::boolean}, {0x85BDA341, ConfigType::boolean},
    {0x38623779, ConfigType::boolean}, {0x1E7432A5, ConfigType::boolean},
    {0x559F78DC, ConfigType::boolean}, {0x0E533025, ConfigType::boolean},
    {0x6834D5DA, ConfigType::boolean}, {0xCBE162F8, ConfigType::boolean},
    {0x553D3C3F, ConfigType::boolean}, {0x89653491, ConfigType::boolean},
    {0x75A591C2, ConfigType::boolean}, {0x325AF11E, ConfigType::boolean},
    {0x6E6CDAD7, ConfigType::boolean}, {0x5DA3152C, ConfigType::boolean},
    {0xF2A389D2, ConfigType::boolean}, {0x3B6A149E, ConfigType::boolean},
    {0xA3B423BE, ConfigType::boolean}, {0x52DAAA5D, ConfigType::boolean},
    {0x081D65D4, ConfigType::boolean}, {0xD13FFDF6, ConfigType::boolean},
    {0x6A586832, ConfigType::boolean}, {0xF8774353, ConfigType::boolean},
    {0xFFD5C89D, ConfigType::boolean}, {0x2802A1F6, ConfigType::boolean},
    {0x2B4CDE08, ConfigType::boolean}, {0x4FD3F678, ConfigType::boolean},
    {0x42694ACE, ConfigType::boolean}, {0x2D915386, ConfigType::boolean},
    {0xA10DC407, ConfigType::color},   {0xB8C2C5BC, ConfigType::color},
    {0x2B61D25A, ConfigType::color},   {0x2ADA481B, ConfigType::color},
    {0x8428F8F9, ConfigType::color},   {0xC2611C1F, ConfigType::color},
    {0xA6B878A8, ConfigType::color},   {0xDB9A269A, ConfigType::color},
    {0x174ABFD6, ConfigType::color},   {0xF407872B, ConfigType::color},
    {0x082CE178, ConfigType::color},   {0x928C112A, ConfigType::color},
    {0xCEE5C756, ConfigType::color},   {0x55436936, ConfigType::color},
    {0x72894F7D, ConfigType::color},   {0xCF605580, ConfigType::color},
    {0x8B5C32AE, ConfigType::color},   {0x76DA764A, ConfigType::color},
    {0x406A4767, ConfigType::color},   {0xC11B6FBD, ConfigType::color},
    {0xB424B6AE, ConfigType::color},   {0xC8555DD4, ConfigType::color},
    {0x58386184, ConfigType::color},   {0x06D6A3C6, ConfigType::color},
    {0xE5F28DFE, ConfigType::color},   {0x6D3D6833, ConfigType::integer},
    {0x59A226C2, ConfigType::integer}, {0x7174D684, ConfigType::hotkey},
};
struct DangerZoneControl {
    std::uint32_t enabled, color;
    std::u16string_view label;
};
constexpr DangerZoneControl danger_zone_controls[] = {
    {0x75A591C2, 0xDB9A269A, u"Drone gun"},
    {0x325AF11E, 0x174ABFD6, u"Blackhawk"},
    {0x6E6CDAD7, 0xF407872B, u"Drone"},
    {0x5DA3152C, 0x082CE178, u"Random case"},
    {0xF2A389D2, 0x928C112A, u"Tool case"},
    {0x3B6A149E, 0xCEE5C756, u"Pistol case"},
    {0xA3B423BE, 0x55436936, u"Explosive case"},
    {0x52DAAA5D, 0x72894F7D, u"Heavy weapon case"},
    {0x081D65D4, 0xCF605580, u"Light weapon case"},
    {0xD13FFDF6, 0x8B5C32AE, u"Dufflebag"},
    {0x6A586832, 0x76DA764A, u"Jammer"},
    {0xF8774353, 0x406A4767, u"Ammobox"},
    {0xFFD5C89D, 0xC11B6FBD, u"Armor"},
    {0x2802A1F6, 0xB424B6AE, u"Parachute pack"},
    {0x2B4CDE08, 0xC8555DD4, u"Briefcase"},
    {0x4FD3F678, 0x58386184, u"Tablet upgrade zone"},
    {0x42694ACE, 0x06D6A3C6, u"Tablet upgrade drone"},
    {0x2D915386, 0xE5F28DFE, u"Cash stack"},
};
}

OtherEspMenu::OtherEspMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Other ESP", {12, 12, 0, 12}, 9, true, identities) {
    require_settings(settings, other_esp_settings);
    storage_.reserve(std::size(other_esp_settings) + 1);
    bindings_.reserve(std::size(other_esp_settings) + 1);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto color = [&](std::uint32_t hash) -> ColorPicker& {
        return add<ColorPicker>(category_, hash, value<ColorConfig>(settings, hash), identities);
    };
    auto tie_pair = [&](Checkbox& owner, std::size_t offset, Widget& first, Widget& second) {
        first.set_visible(false);
        second.set_visible(false);
        dependencies_[offset] = {&first, &dependencies_[offset + 1]};
        dependencies_[offset + 1] = {&second, &owner.tied};
        owner.tied.next = &dependencies_[offset];
    };
    checkbox(0xD7CD1B0B, u"Radar");
    auto& dropped = add<MultiSelect>(category_, 0x6D3D6833, u"Dropped weapons",
                                     value<IntegerConfig>(settings, 0x6D3D6833), identities, false);
    dropped.add_option("Icon", 1);
    dropped.add_option("Text", 2);
    dropped.add_option("Glow", 4);
    dropped.add_option("Ammo", 8);
    dropped.add_option("Distance", 16);
    color(0xA10DC407);
    checkbox(0x25B4B670, u"Grenades");
    color(0xB8C2C5BC);
    checkbox(0xEDF6C582, u"Inaccuracy overlay");
    color(0x2B61D25A);
    checkbox(0x78A1B150, u"Recoil overlay");
    checkbox(0xAD37A2EB, u"Crosshair");
    checkbox(0xF2937E07, u"Bomb");
    color(0x2ADA481B);
    auto& trajectory = checkbox(0xDC4C1BB9, u"Grenade trajectory");
    color(0x8428F8F9);
    auto& hit_label = add<Label>(category_, 0, u"Grenade trajectory (hit)", identities);
    auto& hit_color = color(0xC2611C1F);
    tie_pair(trajectory, 0, hit_label, hit_color);
    checkbox(0x85BDA341, u"Grenade proximity warning");
    checkbox(0x38623779, u"Spectators");
    checkbox(0x1E7432A5, u"Penetration reticle");
    checkbox(0x559F78DC, u"Hostages");
    color(0xA6B878A8);
    auto& indicators =
        add<MultiSelect>(category_, 0x59A226C2, u"Feature indicators",
                         value<IntegerConfig>(settings, 0x59A226C2), identities, false);
    indicators.add_option("Force safe point", 1);
    indicators.add_option("Force body aim", 2);
    indicators.add_option("Ping spike", 4);
    indicators.add_option("Double tap", 8);
    indicators.add_option("Duck peek assist", 16);
    indicators.add_option("Freestanding", 32);
    indicators.add_option("On shot anti-aim", 64);
    indicators.add_option("Minimum damage override", 256);
    auto& shared = checkbox(0x0E533025, u"Shared ESP");
    add<HotkeyWidget>(category_, 0x7174D684, value<HotkeyConfig>(settings, 0x7174D684), true,
                      identities);
    auto& other_team = checkbox(0x6834D5DA, u"Shared ESP with other team");
    auto& restrict_updates = checkbox(0xCBE162F8, u"Restrict shared ESP updates");
    tie_pair(shared, 2, other_team, restrict_updates);
    checkbox(0x553D3C3F, u"Upgrade tablet");
    auto& danger_zone = checkbox(0x89653491, u"Danger Zone items");
    for (const auto& control : danger_zone_controls) {
        checkbox(control.enabled, control.label);
        last_danger_zone_control_ = &color(control.color);
    }
    danger_zone.callbacks.push_back({this, danger_zone_changed}); // 0x433C1019
    danger_zone_changed(this, danger_zone); // 0x4348ABA1..0x4348ABAD
    tab.categories.push_back(&category_);
}
OtherEspMenu::~OtherEspMenu() {
    std::erase(tab_.categories, &category_);
}

void OtherEspMenu::danger_zone_changed(void* context, Widget& source) { // 0x4334D372
    auto& self = *static_cast<OtherEspMenu*>(context);
    auto& checkbox = static_cast<Checkbox&>(source);
    const auto visible = checkbox.value && *checkbox.value != 0;
    auto position =
        std::find(self.category_.elements.begin(), self.category_.elements.end(), &source);
    if (position == self.category_.elements.end())
        return;
    for (++position; position != self.category_.elements.end(); ++position) {
        (*position)->set_visible(visible);
        if (*position == self.last_danger_zone_control_)
            break;
    }
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: players_menu.cpp
// ============================================================================

namespace skeet::ui {
PlayersMenu::PlayersMenu(Tab& tab, WidgetIdentities& identities, players::State& state,
                         players::Bindings& bindings)
    : tab_(tab), players_(&tab, u"Players", {0, 12, 0, 24}, 0, true, identities), state_(state),
      runtime_bindings_(bindings) {
    storage_.reserve(14);
    bindings_.reserve(14);
    tab.categories.push_back(&players_);
    list_ = &add<Listbox>(players_, 0, u"Player list", Point{158, 425}, &selected_player, false,
                          identities);
    list_->callbacks.push_back({this, selection_changed});
    add<Button>(players_, 0, u"Reset all", Point{158, 28}, WidgetCallback{this, reset_clicked},
                identities);
    adjustments_ = std::make_unique<Category>(&tab, u"Adjustments", GridPosition{12, 12, 0, 24}, 13,
                                              true, identities);
    tab.categories.push_back(adjustments_.get());
    auto checkbox = [&](std::u16string_view label) -> Checkbox* {
        return &add<Checkbox>(*adjustments_, 0, label, nullptr, identities);
    };
    whitelist_ = checkbox(u"Add to whitelist");
    shared_ = checkbox(u"Allow shared ESP updates");
    visuals_ = checkbox(u"Disable visuals");
    priority_ = checkbox(u"High priority");
    force_pitch_ = checkbox(u"Force pitch");
    pitch_ = &add<Slider>(*adjustments_, 0, u"Force pitch value", -90, 90, nullptr, true, 0, 1.0f,
                          identities);
    pitch_->maximum_width = 176;
    pitch_->set_visible(false);
    links_[0] = {pitch_, &force_pitch_->tied};
    force_pitch_->tied.next = &links_[0];
    force_body_ = checkbox(u"Force body yaw");
    body_ = &add<Slider>(*adjustments_, 0, u"Force body yaw value", -60, 60, nullptr, true, 0, 1.0f,
                         identities);
    body_->maximum_width = 176;
    body_->set_visible(false);
    links_[1] = {body_, &force_body_->tied};
    force_body_->tied.next = &links_[1];
    correction_ = checkbox(u"Correction active");
    prefer_body_ = &add<ComboBox>(*adjustments_, 0, u"Override prefer body aim",
                                  static_cast<std::uint32_t*>(nullptr), false, identities);
    prefer_body_->set_options({{u"-", 0}, {u"Off", 1}, {u"On", 2}, {u"Force", 3}});
    safe_ = &add<ComboBox>(*adjustments_, 0, u"Override safe point",
                           static_cast<std::uint32_t*>(nullptr), false, identities);
    safe_->set_options({{u"-", 0}, {u"Off", 1}, {u"On", 2}});
    add<Button>(*adjustments_, 0, u"Apply to all", Point{158, 28},
                WidgetCallback{this, apply_clicked}, identities);
}
PlayersMenu::~PlayersMenu() {
    std::erase(tab_.categories, &players_);
    std::erase(tab_.categories, adjustments_.get());
}
void PlayersMenu::enable_category(Category& category, bool enabled) {
    for (auto* control : std::span(category.elements))
        control->enabled = enabled;
}
void PlayersMenu::activate() {
    const auto list =
        players::build_list(state_, runtime_bindings_, list_->value ? *list_->value : 0);
    if (!list.active) {
        for (auto* category : std::span(tab_.categories))
            enable_category(*category, false);
        list_->set_options(nullptr);
        return;
    }
    std::vector<ListboxOption> options;
    options.reserve(list.entries.size());
    for (const auto& entry : list.entries)
        options.push_back({entry.player_index, entry.name});
    list_->set_options(&options);
    if (list_->options.empty()) {
        for (auto* category : std::span(tab_.categories))
            enable_category(*category, false);
        return;
    }
    enable_category(players_, true);
    list_->set_selection(list.selected_row);
    const auto selected = list_->options[std::size_t(list_->selected_item)].value;
    if (list_->value)
        *list_->value = selected;
    auto& value = state_.records[std::size_t(selected)].adjustments;
    whitelist_->value = &value.whitelist;
    whitelist_->config_loaded();
    shared_->value = list.shared_esp_enabled ? &value.allow_shared_esp : nullptr;
    shared_->config_loaded();
    visuals_->value = &value.disable_visuals;
    visuals_->config_loaded();
    priority_->value = &value.high_priority;
    priority_->config_loaded();
    force_pitch_->value = &value.force_pitch;
    force_pitch_->config_loaded();
    pitch_->value = &value.pitch;
    pitch_->config_loaded();
    force_body_->value = &value.force_body_yaw;
    force_body_->config_loaded();
    body_->value = &value.body_yaw;
    body_->config_loaded();
    correction_->value = &value.correction_active;
    correction_->config_loaded();
    prefer_body_->value = &value.prefer_body_aim;
    prefer_body_->config_loaded();
    safe_->value = &value.safe_point;
    safe_->config_loaded();
    enable_category(*adjustments_, true);
    shared_->enabled = list.shared_esp_enabled;
}
void PlayersMenu::reset_all() {
    if (players::reset_all(state_, runtime_bindings_))
        pitch_->update_value_text();
}
void PlayersMenu::apply_to_all() {
    players::apply_to_all(state_, runtime_bindings_,
                          list_->value ? static_cast<std::uint32_t>(*list_->value) : 0);
}
void PlayersMenu::selection_changed(void* context, Widget&) {
    static_cast<PlayersMenu*>(context)->activate();
}
void PlayersMenu::reset_clicked(void* context, Widget&) {
    static_cast<PlayersMenu*>(context)->reset_all();
}
void PlayersMenu::apply_clicked(void* context, Widget&) {
    static_cast<PlayersMenu*>(context)->apply_to_all();
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: player_esp_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement player_esp_settings[] = {
    {0x4700A298, ConfigType::hotkey},  {0x45D9D2AC, ConfigType::boolean},
    {0xC410C5D3, ConfigType::boolean}, {0xCFF9EBAD, ConfigType::boolean},
    {0x4D243E0D, ConfigType::color},   {0x49733D3C, ConfigType::boolean},
    {0xD2E84B64, ConfigType::color},   {0xA1C152A1, ConfigType::boolean},
    {0x25FEB6C1, ConfigType::color},   {0x084A8B33, ConfigType::boolean},
    {0xBB3FDF6A, ConfigType::boolean}, {0x6A874B2C, ConfigType::boolean},
    {0x00F64778, ConfigType::color},   {0x43F02F68, ConfigType::boolean},
    {0x8BEC37F4, ConfigType::color},   {0x0F6D81D5, ConfigType::boolean},
    {0xB1D008B7, ConfigType::boolean}, {0xECE872CB, ConfigType::color},
    {0x9C345DFA, ConfigType::boolean}, {0x69C9D9F2, ConfigType::boolean},
    {0x535095F5, ConfigType::boolean}, {0x7451E155, ConfigType::color},
    {0xE9CC8B95, ConfigType::boolean}, {0xE20F6035, ConfigType::color},
    {0xE8D2A207, ConfigType::boolean}, {0x7E3B4C1B, ConfigType::color},
    {0x1DE914E6, ConfigType::boolean}, {0x745EFA1E, ConfigType::color},
    {0x53970838, ConfigType::boolean}, {0x523E4B35, ConfigType::boolean},
    {0x0E995F95, ConfigType::color},   {0x95A660AB, ConfigType::boolean},
    {0x7D18A71F, ConfigType::color},   {0x4564BFC7, ConfigType::integer},
    {0x09CBEDAE, ConfigType::integer},
};
}

PlayerEspMenu::PlayerEspMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Player ESP", {0, 12, 0, 13}, 17, true, identities) {
    require_settings(settings, player_esp_settings);
    storage_.reserve(std::size(player_esp_settings) + 1);
    bindings_.reserve(std::size(player_esp_settings) + 1);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto color = [&](std::uint32_t hash) {
        add<ColorPicker>(category_, hash, value<ColorConfig>(settings, hash), identities);
    };
    add<Label>(category_, 0, u"Activation type", identities);
    add<HotkeyWidget>(category_, 0x4700A298, value<HotkeyConfig>(settings, 0x4700A298), true,
                      identities);
    checkbox(0x45D9D2AC, u"Teammates");
    checkbox(0xC410C5D3, u"Dormant");
    checkbox(0xCFF9EBAD, u"Bounding box");
    color(0x4D243E0D);
    checkbox(0x49733D3C, u"Health bar");
    color(0xD2E84B64);
    checkbox(0xA1C152A1, u"Name");
    color(0x25FEB6C1);
    checkbox(0x084A8B33, u"Flags");
    checkbox(0xBB3FDF6A, u"Weapon text");
    checkbox(0x6A874B2C, u"Weapon icon");
    color(0x00F64778);
    checkbox(0x43F02F68, u"Ammo");
    color(0x8BEC37F4);
    checkbox(0x0F6D81D5, u"Distance");
    checkbox(0xB1D008B7, u"Glow");
    color(0xECE872CB);
    checkbox(0x9C345DFA, u"Hit marker");
    checkbox(0x69C9D9F2, u"Hit marker sound");
    checkbox(0x535095F5, u"Visualize aimbot");
    color(0x7451E155);
    checkbox(0xE9CC8B95, u"Visualize aimbot (safe point)");
    color(0xE20F6035);
    checkbox(0xE8D2A207, u"Visualize sounds");
    color(0x7E3B4C1B);
    checkbox(0x1DE914E6, u"Line of sight");
    color(0x745EFA1E);
    checkbox(0x53970838, u"Money");
    checkbox(0x523E4B35, u"Skeleton");
    color(0x0E995F95);
    auto& arrow = checkbox(0x95A660AB, u"Out of FOV arrow");
    color(0x7D18A71F);
    arrow_size_ =
        &add<Slider>(category_, 0x4564BFC7, u"", 1, 30, value<IntegerConfig>(settings, 0x4564BFC7),
                     true, 0x780070u, 1.0f, identities);
    arrow_size_->set_visible(false);
    arrow_distance_ =
        &add<Slider>(category_, 0x09CBEDAE, u"", 1, 100, value<IntegerConfig>(settings, 0x09CBEDAE),
                     true, 0x25u, 1.0f, identities);
    arrow_distance_->set_visible(false);
    arrow.callbacks.push_back({this, arrow_visibility_changed}); // 0x4335BD4D
    tab.categories.push_back(&category_);
}
PlayerEspMenu::~PlayerEspMenu() {
    std::erase(tab_.categories, &category_);
}

void PlayerEspMenu::arrow_visibility_changed(void* context, Widget& source) { // 0x43330832
    auto& self = *static_cast<PlayerEspMenu*>(context);
    auto& checkbox = static_cast<Checkbox&>(source);
    const auto visible = checkbox.value && *checkbox.value != 0;
    self.arrow_size_->set_visible(visible);
    self.arrow_distance_->set_visible(visible);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: quick_peek_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement quick_peek_settings[] = {
    {0x141C9037, ConfigType::boolean}, {0xEC178029, ConfigType::hotkey},
    {0x4F67FC33, ConfigType::integer}, {0xC9FCBE4B, ConfigType::color},
    {0x4CB8E4AA, ConfigType::integer},
};
}

QuickPeekMenu::QuickPeekMenu(Category& category, ConfigRegistries& settings,
                             WidgetIdentities& identities)
    : category_(category) {
    require_settings(settings, quick_peek_settings);
    storage_.reserve(std::size(quick_peek_settings));
    bindings_.reserve(std::size(quick_peek_settings));
    auto& checkbox = add<Checkbox>(category_, 0x141C9037, u"Quick peek assist",
                                   value<BooleanConfig>(settings, 0x141C9037), identities);
    add<HotkeyWidget>(category_, 0xEC178029, value<HotkeyConfig>(settings, 0xEC178029), true,
                      identities);
    auto& mode = add<MultiSelect>(category_, 0x4F67FC33, u"Quick peek assist mode",
                                  value<IntegerConfig>(settings, 0x4F67FC33), identities);
    mode.add_option("Retreat on shot", 1);
    mode.add_option("Retreat on key release", 2);
    mode.set_visible(false);
    auto& color = add<ColorPicker>(category_, 0xC9FCBE4B, value<ColorConfig>(settings, 0xC9FCBE4B),
                                   identities);
    color.set_visible(false);
    auto& distance =
        add<Slider>(category_, 0x4CB8E4AA, u"Quick peek assist distance", 16, 201,
                    value<IntegerConfig>(settings, 0x4CB8E4AA), true, 0x6E0069u, 1.0f, identities);
    distance.value_labels.emplace(201, L"\u221E");
    if (*distance.value == 201)
        distance.update_value_text(); // 0x43402B1B..0x43402B2B
    distance.set_visible(false);
    dependencies_[0] = {&mode, &dependencies_[1]};
    dependencies_[1] = {&color, &dependencies_[2]};
    dependencies_[2] = {&distance, &checkbox.tied};
    checkbox.tied.next = &dependencies_[0];
}
QuickPeekMenu::~QuickPeekMenu() {
    detach(category_);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: rage_clipboard.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::ui {
namespace {
bool open(void*, std::uintptr_t owner) {
    return OpenClipboard(reinterpret_cast<HWND>(owner)) != FALSE;
}
bool empty(void*) {
    return EmptyClipboard() != FALSE;
}
std::uintptr_t allocate(void*, std::uint32_t flags, std::size_t size) {
    return reinterpret_cast<std::uintptr_t>(GlobalAlloc(flags, size));
}
std::uintptr_t set_data(void*, std::uint32_t format, std::uintptr_t memory) {
    return reinterpret_cast<std::uintptr_t>(
        SetClipboardData(format, reinterpret_cast<HANDLE>(memory)));
}
void free_memory(void*, std::uintptr_t memory) {
    GlobalFree(reinterpret_cast<HGLOBAL>(memory));
}
void close(void*) {
    CloseClipboard();
}
bool available(void*, std::uint32_t format) {
    return IsClipboardFormatAvailable(format) != FALSE;
}
std::uintptr_t get_data(void*, std::uint32_t format) {
    return reinterpret_cast<std::uintptr_t>(GetClipboardData(format));
}
const char* wire(void*, std::uintptr_t memory) {
    return static_cast<const char*>(GlobalWire(reinterpret_cast<HGLOBAL>(memory)));
}
std::size_t size(void*, std::uintptr_t memory) {
    return GlobalSize(reinterpret_cast<HGLOBAL>(memory));
}
void unlock(void*, std::uintptr_t memory) {
    GlobalUnlock(reinterpret_cast<HGLOBAL>(memory));
}

void switch_group(void* raw, bool force) {
    auto& context = *static_cast<RageWeaponClipboardContext*>(raw);
    if (force)
        context.state.last_group = UINT32_MAX;
    switch_rage_weapon_group(context.state, context.selected_group);
}
void copy_group(void* raw, std::uint32_t group) {
    auto& context = *static_cast<RageWeaponClipboardContext*>(raw);
    write_rage_clipboard(context.state.groups[group]->value, *context.clipboard);
}
bool paste_group(void* raw, std::uint32_t group) {
    auto& context = *static_cast<RageWeaponClipboardContext*>(raw);
    ConfigBytes value;
    read_rage_clipboard(value, *context.clipboard);
    if (value.empty())
        return false;
    context.state.groups[group]->value = std::move(value);
    return true;
}
}
RageClipboardApi& system_rage_clipboard() noexcept {
    static RageClipboardApi api{nullptr, open,      empty,    allocate, set_data, free_memory,
                                close,   available, get_data, wire,     size,     unlock};
    return api;
}
void write_rage_clipboard(std::span<const std::byte> value, RageClipboardApi& api) { // 0x4335C6C2
    if (value.empty())
        return;
    const auto encoded = encode_config_export(value);
    if (encoded.empty() || !api.open(api.context, 0))
        return;
    if (api.empty(api.context)) {
        const auto memory = api.allocate(api.context, 0, encoded.size() + 1);
        if (memory) {
            std::memcpy(reinterpret_cast<void*>(memory), encoded.c_str(), encoded.size() + 1);
            if (!api.set_data(api.context, 1, memory))
                api.free(api.context, memory);
        }
    }
    api.close(api.context);
}
void read_rage_clipboard(ConfigBytes& destination, RageClipboardApi& api) { // 0x43394985
    if (!api.available(api.context, 1) || !api.open(api.context, 0))
        return;
    if (const auto memory = api.get_data(api.context, 1)) {
        if (const auto text = api.wire(api.context, memory)) {
            // The native routine passes the locked pointer to both APIs.
            const auto pointer = reinterpret_cast<std::uintptr_t>(text);
            const auto capacity = api.size(api.context, pointer);
            std::size_t length = 0;
            while (length < capacity && text[length])
                ++length;
            destination = decode_config_export(std::string_view(text, length));
            api.unlock(api.context, pointer);
        }
    }
    api.close(api.context);
}
RageWeaponSelectorActions
rage_weapon_clipboard_actions(RageWeaponClipboardContext& context) noexcept {
    return {&context, switch_group, copy_group, paste_group};
}
void RageWeaponEnabledSettings::register_config(ConfigLink& registry) noexcept {
    for (std::size_t i = 0; i < values.size(); ++i)
        initialize_boolean_config(registry, values[i], hashes[i]);
}
std::array<BooleanConfig*, 12> RageWeaponEnabledSettings::descriptors() noexcept {
    std::array<BooleanConfig*, 12> result;
    for (std::size_t i = 0; i < values.size(); ++i)
        result[i] = &values[i];
    return result;
}
void RageWeaponEnabledSettings::bind(RageWeaponSelector& selector) noexcept {
    auto pointers = descriptors();
    selector.bind_enabled_groups(pointers);
}
}

// ============================================================================
// Subcomponent: rage_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement rage_requirements[] = {
    {0x23060EC5, ConfigType::boolean}, {0x861E7F9A, ConfigType::hotkey},
    {0x65E86322, ConfigType::integer}, {0x5B900854, ConfigType::integer},
    {0xBFABB4A0, ConfigType::integer}, {0x29E2C8ED, ConfigType::hotkey},
    {0xA25029BA, ConfigType::integer}, {0x3F5E5553, ConfigType::integer},
    {0x18C0E6BE, ConfigType::integer}, {0xB80303B4, ConfigType::boolean},
    {0x7EB3A142, ConfigType::hotkey},  {0x7DC413FE, ConfigType::integer},
    {0xF5A00B3B, ConfigType::boolean}, {0x333C1DBE, ConfigType::hotkey},
    {0xECBBD134, ConfigType::integer}, {0x2B54548D, ConfigType::boolean},
    {0x16C91A85, ConfigType::integer}, {0x02F59674, ConfigType::hotkey},
    {0x9BBD48EA, ConfigType::boolean}, {0xC06FD8CC, ConfigType::boolean},
    {0x1AC79342, ConfigType::hotkey},  {0x11195AD9, ConfigType::integer},
    {0x0889939F, ConfigType::boolean}, {0x67C9986D, ConfigType::hotkey},
    {0x31E2D7DD, ConfigType::integer}, {0x3F69C17C, ConfigType::integer},
    {0x2B56E048, ConfigType::integer}, {0x9FBDD0E6, ConfigType::integer},
    {0x9E2B2E76, ConfigType::boolean},
};
void hitboxes(MultiSelect& control) {
    control.add_option("Head", 2);
    control.add_option("Chest", 4);
    control.add_option("Stomach", 8);
    control.add_option("Arms", 0x30);
    control.add_option("Legs", 0xC0);
    control.add_option("Feet", 0x200);
}
}
RageAimbotMenu::RageAimbotMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Aimbot", {0, 12, 0, 24}, 24, true, identities) {
    require_settings(settings, rage_requirements);
    tab.categories.push_back(&category_); //43487180, before the first bound control.
    category_.movable = category_.resizable = false;
    storage_.reserve(31);
    bindings_.reserve(31);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto key = [&](std::uint32_t hash) -> HotkeyWidget& {
        return add<HotkeyWidget>(category_, hash, value<HotkeyConfig>(settings, hash), true,
                                 identities);
    };
    auto multi = [&](std::uint32_t hash, std::u16string_view label,
                     bool require_one = false) -> MultiSelect& {
        return add<MultiSelect>(category_, hash, label, value<IntegerConfig>(settings, hash),
                                identities, require_one);
    };
    auto slider = [&](std::uint32_t hash, std::u16string_view label, int minimum, int maximum,
                      std::uint32_t suffix = 0) -> Slider& {
        return add<Slider>(category_, hash, label, minimum, maximum,
                           value<IntegerConfig>(settings, hash), true, suffix, 1.0f, identities);
    };
    auto tie = [&](std::size_t index, Checkbox& source, Widget& target) {
        target.set_visible(false);
        dependencies_[index] = {&target, &source.tied};
        auto* end = &source.tied;
        while (end->next != &source.tied)
            end = end->next;
        end->next = &dependencies_[index];
    };
    checkbox(0x23060EC5, u"Enabled");
    key(0x861E7F9A);
    auto& selection = add<ComboBox>(category_, 0x65E86322, u"Target selection",
                                    value<IntegerConfig>(settings, 0x65E86322), false, identities);
    selection.set_options({{u"Highest damage", 2},
                           {u"Cycle", 0},
                           {u"Cycle (2x)", 3},
                           {u"Near crosshair", 1},
                           {u"Best hit chance", 6}});
    auto& selected = multi(0x5B900854, u"Target hitbox", true);
    hitboxes(selected);
    selected.callbacks.push_back({this, hitboxes_changed});
    auto& points = multi(0xBFABB4A0, u"Multi-point");
    hitboxes(points);
    points.callbacks.push_back({this, multipoint_changed});
    key(0x29E2C8ED);
    multipoint_scale_ = &slider(0xA25029BA, u"Multi-point scale", 24, 100, 0x25);
    multipoint_scale_->set_visible(false);
    multipoint_scale_->value_labels.emplace(24, L"Auto");
    if (*multipoint_scale_->value == 24)
        multipoint_scale_->update_value_text();
    auto& chance = slider(0x3F5E5553, u"Minimum hit chance", 0, 100, 0x25);
    chance.value_labels.emplace(0, L"Off");
    if (*chance.value == 0)
        chance.update_value_text();
    add<RageDamageSlider>(category_, 0x18C0E6BE, u"Minimum damage",
                          value<IntegerConfig>(settings, 0x18C0E6BE), identities);
    auto& override = checkbox(0xB80303B4, u"Minimum damage override");
    key(0x7EB3A142);
    auto& override_damage = add<RageDamageSlider>(
        category_, 0x7DC413FE, u"", value<IntegerConfig>(settings, 0x7DC413FE), identities);
    tie(0, override, override_damage);
    checkbox(0xF5A00B3B, u"Prefer safe point");
    add<Label>(category_, 0, u"Force safe point", identities);
    key(0x333C1DBE);
    auto& unsafe = multi(0xECBBD134, u"Avoid unsafe hitboxes");
    hitboxes(unsafe);
    prefer_body_ = &checkbox(0x2B54548D, u"Prefer body aim");
    prefer_body_->set_visible(false);
    auto& disablers = multi(0x16C91A85, u"Prefer body aim disablers");
    disablers.add_option("Low inaccuracy", 1);
    disablers.add_option("Target shot fired", 2);
    disablers.add_option("Target resolved", 4);
    disablers.add_option("Safe point headshot", 8);
    tie(1, *prefer_body_, disablers);
    add<Label>(category_, 0, u"Force body aim", identities);
    key(0x02F59674);
    checkbox(0x9BBD48EA, u"Force body aim on peek");
    auto& stop = checkbox(0xC06FD8CC, u"Quick stop");
    key(0x1AC79342);
    auto& stop_options = multi(0x11195AD9, u"");
    stop_options.add_option("Early", 1);
    stop_options.add_option("Slow motion", 4);
    stop_options.add_option("Duck", 2);
    stop_options.add_option("Fake duck", 0x20);
    stop_options.add_option("Move between shots", 8);
    stop_options.add_option("Ignore molotov", 0x10);
    stop_options.add_option("Taser", 0x40);
    stop_options.add_option("Jump scout", 0x80);
    tie(2, stop, stop_options);
    auto& double_tap = checkbox(0x0889939F, u"Double tap");
    double_tap.color = 0xBB82F0F0;
    key(0x67C9986D);
    auto& mode = add<ComboBox>(category_, 0x31E2D7DD, u"",
                               value<IntegerConfig>(settings, 0x31E2D7DD), false, identities);
    mode.set_options({{u"Offensive", 0}, {u"Defensive", 1}});
    tie(3, double_tap, mode);
    tie(4, double_tap, slider(0x3F69C17C, u"Double tap hit chance", 0, 100, 0x25));
    tie(5, double_tap, slider(0x2B56E048, u"Double tap fake lag limit", 1, 10));
    auto& double_stop = multi(0x9FBDD0E6, u"Double tap quick stop");
    double_stop.add_option("Slow motion", 4);
    double_stop.add_option("Duck", 2);
    double_stop.add_option("Move between shots", 8);
    tie(6, double_tap, double_stop);
    checkbox(0x9E2B2E76, u"Automatic scope");
}
RageAimbotMenu::~RageAimbotMenu() {
    std::erase(tab_.categories, &category_);
}
void RageAimbotMenu::hitboxes_changed(void* context, Widget& source) { // 0x433CEE44 / 0x433EFA51
    auto& self = *static_cast<RageAimbotMenu*>(context);
    auto* selected = static_cast<MultiSelect&>(source).value;
    const auto mask = selected ? *selected : 0;
    const bool allowed = (mask & 2) && (mask & 0xE) != 2;
    if (!allowed)
        self.prefer_body_->set_value(false);
    self.prefer_body_->set_visible(allowed);
}
void RageAimbotMenu::multipoint_changed(void* context, Widget& source) { // 0x43348202 / 0x43369A9B
    auto* selected = static_cast<MultiSelect&>(source).value;
    static_cast<RageAimbotMenu*>(context)->multipoint_scale_->set_visible(selected &&
                                                                          *selected != 0);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: rage_other_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement rage_other_requirements[] = {
    {0x4346EDB5, ConfigType::integer}, {0xA4D31376, ConfigType::boolean},
    {0x59BD212A, ConfigType::boolean}, {0x64715C40, ConfigType::boolean},
    {0x86BFE317, ConfigType::boolean}, {0x63660060, ConfigType::boolean},
    {0xA09C0D52, ConfigType::boolean}, {0xD23ED2A7, ConfigType::hotkey},
    {0x4034958C, ConfigType::boolean}, {0x20E889DE, ConfigType::integer},
    {0xD6D22A5A, ConfigType::integer}, {0xE82BE8DD, ConfigType::boolean},
    {0x2B385DC7, ConfigType::integer}, {0x141C9037, ConfigType::boolean},
    {0xEC178029, ConfigType::hotkey},  {0x4F67FC33, ConfigType::integer},
    {0xC9FCBE4B, ConfigType::color},   {0x4CB8E4AA, ConfigType::integer},
};
}
RageOtherMenu::RageOtherMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Other", {12, 12, 0, 24}, 8, true, identities) {
    require_settings(settings, rage_other_requirements);
    tab.categories.push_back(&category_); //43487B42, before the first bound control.
    category_.movable = category_.resizable = false;
    storage_.reserve(14);
    bindings_.reserve(19);
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto& accuracy = add<ComboBox>(category_, 0x4346EDB5, u"Accuracy boost",
                                   value<IntegerConfig>(settings, 0x4346EDB5), false, identities);
    accuracy.set_options({{u"Low", 1}, {u"Medium", 3}, {u"High", 2}, {u"Maximum", 4}});
    checkbox(0xA4D31376, u"Anti-aim correction");
    checkbox(0x59BD212A, u"Automatic fire");
    checkbox(0x64715C40, u"Automatic penetration");
    checkbox(0x86BFE317, u"Silent aim");
    checkbox(0x63660060, u"Remove recoil");
    checkbox(0xA09C0D52, u"Delay shot");
    quick_peek_ = std::make_unique<QuickPeekMenu>(category_, settings, identities);
    const auto peek_bindings = quick_peek_->controls();
    bindings_.insert(bindings_.end(), peek_bindings.begin(), peek_bindings.end());
    add<Label>(category_, 0, u"Duck peek assist", identities);
    add<HotkeyWidget>(category_, 0xD23ED2A7, value<HotkeyConfig>(settings, 0xD23ED2A7), true,
                      identities);
    checkbox(0x4034958C, u"Reduce aim step");
    add<Slider>(category_, 0x20E889DE, u"Reduce aim step limit", 2, 29,
                value<IntegerConfig>(settings, 0x20E889DE), true, 0xB0, 1.0f, identities)
        .set_visible(false);
    add<Slider>(category_, 0xD6D22A5A, u"Maximum FOV", 1, 180,
                value<IntegerConfig>(settings, 0xD6D22A5A), true, 0xB0, 1.0f, identities);
    checkbox(0xE82BE8DD, u"Log misses due to spread");
    auto& mitigations =
        add<MultiSelect>(category_, 0x2B385DC7, u"Low FPS mitigations",
                         value<IntegerConfig>(settings, 0x2B385DC7), identities, false);
    mitigations.add_option("Force low accuracy boost", 1);
    mitigations.add_option("Disable multipoint: feet", 2);
    mitigations.add_option("Disable multipoint: arms", 4);
    mitigations.add_option("Disable multipoint: legs", 8);
    mitigations.add_option("Disable hitbox: feet", 0x10);
    mitigations.add_option("Lower hit chance precision", 0x40);
    mitigations.add_option("Limit targets per tick", 0x80);
}
RageOtherMenu::~RageOtherMenu() {
    std::erase(tab_.categories, &category_);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: rage_weapon_groups.cpp
// ============================================================================

namespace skeet::ui {
void switch_rage_weapon_group(RageWeaponGroupState& state, const std::uint32_t* selected_group) {
    const auto previous = state.last_group;
    const auto selected = selected_group ? *selected_group : 0u;
    if (selected >= 12 || selected == previous)
        return;

    if (previous < 12) {
        ConfigBytes changes;
        save_category_config(state.window, state.registry, "rage", "aimbot", changes,
                             state.baseline_a.bytes(), state.baseline_b.bytes());
        // The original deliberately retains the stored group when the current
        // values serialize to the header alone; it does not clear that group.
        if (changes.size() > 12)
            state.groups[previous]->value = changes;
    }
    load_category_config(state.window, state.registry, "rage", "aimbot",
                         state.groups[selected]->value, state.baseline_a.bytes(),
                         state.baseline_b.bytes());
    state.last_group = selected;
}
}

// ============================================================================
// Subcomponent: rage_weapon_menu.cpp
// ============================================================================

namespace skeet::ui {
RageWeaponMenu::RageWeaponMenu(Tab& tab, std::uint32_t& selected_group,
                               std::span<std::uint8_t> enabled_groups,
                               RageWeaponSelectorActions actions, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Weapon type", {0, 12, 0, -1}, 1, false, identities) {
    if (enabled_groups.size() < 12)
        throw std::invalid_argument("RAGE selector requires twelve weapon groups");
    tab.categories.push_back(&category_); //43487103, before selector construction.
    category_.fixed_height = category_.default_fixed_height = 50;
    category_.resizable = category_.movable = false;
    selector_ = &add<RageWeaponSelector>(category_, 0, u"Weapon type", &selected_group, identities,
                                         enabled_groups, actions);
}
RageWeaponMenu::RageWeaponMenu(Tab& tab, std::uint32_t& selected_group,
                               std::span<BooleanConfig*, 12> enabled_groups,
                               RageWeaponSelectorActions actions, WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Weapon type", {0, 12, 0, -1}, 1, false, identities) {
    tab.categories.push_back(&category_); //43487103, before selector construction.
    category_.fixed_height = category_.default_fixed_height = 50;
    category_.resizable = category_.movable = false;
    selector_ = &add<RageWeaponSelector>(category_, 0, u"Weapon type", &selected_group, identities,
                                         enabled_groups, actions);
}
RageWeaponMenu::~RageWeaponMenu() {
    std::erase(tab_.categories, &category_);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: rage_weapon_selector.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup add/sub/mul] */
constexpr std::array<std::uint8_t, 12> symbols{0, 61, 19, 7, 27, 28, 38, 9, 40, 1, 64, 31};
std::uint8_t& group_flag(const std::array<std::uint8_t*, 12>& groups, std::uint32_t group) {
    if (group >= groups.size() || !groups[group])
        throw std::out_of_range("RAGE selector requires its weapon-group flag");
    return *groups[group];
}
}
RageWeaponSelector::RageWeaponSelector() {
    context_menu.parent = this;
    context_menu.selected = select_context;
}
RageWeaponSelector::RageWeaponSelector(Widget* owner, std::u16string_view label,
                                       std::uint32_t* selection, WidgetIdentities& identities,
                                       std::span<std::uint8_t> groups,
                                       RageWeaponSelectorActions callbacks) // 0x4348E18F
    : ComboBox(owner, label, selection, true, identities), actions(callbacks),
      context_menu(this, {u"Copy", u"Paste"}, identities) {
    bind_enabled_groups(groups);
    initialize_selector();
}
RageWeaponSelector::RageWeaponSelector(Widget* owner, std::u16string_view label,
                                       std::uint32_t* selection, WidgetIdentities& identities,
                                       std::span<BooleanConfig*, 12> groups,
                                       RageWeaponSelectorActions callbacks)
    : ComboBox(owner, label, selection, true, identities), actions(callbacks),
      context_menu(this, {u"Copy", u"Paste"}, identities) {
    bind_enabled_groups(groups);
    initialize_selector();
}
void RageWeaponSelector::initialize_selector() {
    context_menu.selected = select_context;
    geometry.bounds = {0, 0, 214, 46};
    geometry.default_size = {214, 46};
    geometry.horizontal_offset = 0;
    default_row_height = row_height = 30;
    set_options({{u"Global", 0},
                 {u"G3SG1 / SCAR-20", 6},
                 {u"SSG 08", 8},
                 {u"AWP", 7},
                 {u"R8 Revolver", 10},
                 {u"Desert Eagle", 9},
                 {u"Pistol", 1},
                 {u"Zeus", 11},
                 {u"Rifle", 3},
                 {u"Shotgun", 4},
                 {u"SMG", 2},
                 {u"Machine gun", 5}});
}
void RageWeaponSelector::bind_enabled_groups(std::span<std::uint8_t> values) {
    if (values.size() < enabled_groups.size())
        throw std::invalid_argument("RAGE selector requires twelve weapon-group flags");
    for (std::size_t i = 0; i < enabled_groups.size(); ++i)
        enabled_groups[i] = &values[i];
}
void RageWeaponSelector::bind_enabled_groups(std::span<BooleanConfig*, 12> descriptors) {
    for (std::size_t i = 0; i < enabled_groups.size(); ++i) {
        if (!descriptors[i])
            throw std::invalid_argument("RAGE selector requires a live BooleanConfig descriptor");
        enabled_groups[i] = &descriptors[i]->value;
    }
}
bool RageWeaponSelector::hit_test(Point cursor) const { // 0x433FBC33
    return context_menu.hit_test(cursor) || ComboBox::hit_test(cursor);
}
void RageWeaponSelector::reset_input() { // 0x433D1EB9
    ComboBox::reset_input();
    context_menu.reset_input();
}
void RageWeaponSelector::value_changed() { // 0x4332A7C8
    ComboBox::value_changed();
    if (!actions.switch_group)
        throw std::logic_error("RAGE selector group store is not installed");
    actions.switch_group(actions.context, false);
}
void RageWeaponSelector::resize(const SliderMetrics&) { // 0x43381FC8
    if (!parent)
        throw std::logic_error("RAGE selector layout requires its category");
    geometry.bounds.width = parent->geometry.bounds.width;
    geometry.bounds.height = parent->geometry.bounds.height;
    update_scroll();
}
void RageWeaponSelector::dpi_changed(float factor, const SliderMetrics& metrics) { // 0x433F2A28
    ComboBox::dpi_changed(factor, metrics);
    if (!parent)
        throw std::logic_error("RAGE selector layout requires its category");
    geometry.bounds.width = parent->geometry.bounds.width;
    geometry.bounds.height = parent->geometry.bounds.height;
    icon_size = scale_size({76, 20}, factor);
    global_icon_size = round_layout_coordinate(factor * 32.0f);
    checkbox_size = round_layout_coordinate(factor * 8.0f);
    context_menu.dpi_changed(factor, metrics);
}
void RageWeaponSelector::on_layout() { // 0x43330C8A
    geometry.bounds.x = geometry.horizontal_offset;
    geometry.bounds.y = 0;
}
void RageWeaponSelector::interact_options(const InputMessage& message, Point cursor) { // 0x4337FEBF
    const auto y = sub(sub(cursor.y, geometry.bounds.height), geometry.bounds.y);
    if (y < 0)
        hovered_item = -1;
    else {
        const auto offset = scroll.enabled ? sub(scroll.position, scroll.position % row_height) : 0;
        const auto index = std::uint32_t(add(offset, y) / row_height);
        if (index < options.size()) {
            hovered_item = y / row_height;
            if (held) {
                const auto group = options[index].value;
                if (cursor.x < round_layout_coordinate(scale * 50.0f)) {
                    if (message.message != 0x202)
                        return;
                    if (group)
                        group_flag(enabled_groups, group) = group_flag(enabled_groups, group) == 0;
                    held = false;
                    return;
                }
                if ((value ? *value : 0) != group)
                    set_value(group);
            }
        }
    }
    if (message.message == 0x202)
        open = held = false;
}
bool RageWeaponSelector::combo_mouse(const InputMessage& message, Point cursor) {
    // 0x4340C49A dispatches its option interaction through virtual slot 0x34.
    if (!enabled)
        return false;
    if (open && scroll.enabled &&
        scrollbar_mouse(
            scroll, message.message,
            {sub(cursor.x, geometry.bounds.x), sub(sub(cursor.y, row_height), geometry.bounds.y)},
            scale))
        return true;
    if (message.message == 0x200) {
        if (!hit_test(cursor)) {
            hovered_item = -1;
            hovered = held = false;
            return false;
        }
        hovered = true;
        if (open)
            interact_options(message, cursor);
        return held;
    }
    if (message.message == 0x201) {
        if (!hit_test(cursor))
            return false;
        held = true;
        return true;
    }
    if (message.message == 0x202) {
        scroll.dragging = false;
        if (!held)
            return false;
        if (hit_test(cursor)) {
            if (!open) {
                open = true;
                held = false;
            } else
                interact_options(message, cursor);
            return true;
        }
        held = false;
        return false;
    }
    if (message.message == 0x204 || message.message == 0x205)
        return hit_test(cursor);
    if (message.message == 0x20a && open && scroll.enabled && hit_test(cursor)) {
        const auto wheel = std::bit_cast<std::int16_t>(std::uint16_t(message.parameter >> 16));
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height,
                             mul(row_height, wheel > 0 ? -4 : 4));
        return true;
    }
    return false;
}
bool RageWeaponSelector::mouse(const InputMessage& message, Point cursor) { // 0x433EA5E2
    bool handled = false;
    bool blocked = false;
    if (context_menu.open) {
        if (context_menu.mouse(message, cursor))
            return true;
        blocked = context_menu.open && context_menu.hit_test(cursor);
    }
    if (!blocked) {
        handled = combo_mouse(message, cursor);
        if (handled) {
            if (message.message != 0x200)
                context_menu.reset_input();
            if (message.message == 0x205) {
                const auto y = sub(cursor.y, add(geometry.bounds.height, geometry.bounds.y));
                bool valid = false;
                if (y < 0) {
                    if (ComboBox::hit_test(cursor) && !options.empty()) {
                        context_group = value ? *value : 0;
                        valid = true;
                    }
                } else {
                    const auto offset =
                        scroll.enabled ? sub(scroll.position, scroll.position % row_height) : 0;
                    const auto index = std::uint32_t(add(offset, y) / row_height);
                    if (index < options.size()) {
                        context_group = options[index].value;
                        valid = true;
                    }
                }
                if (valid) {
                    context_menu.geometry.bounds.x =
                        add(context_menu.geometry.horizontal_offset, cursor.x);
                    context_menu.geometry.bounds.y = cursor.y;
                    context_menu.open = true;
                }
            }
        }
    }
    auto* category = owning_category(this);
    if (!category)
        throw std::logic_error("RAGE selector input requires its category");
    category->background = hovered || open ? 0xff191919 : 0xff171717;
    return handled;
}
void RageWeaponSelector::select_context(Widget& owner, PopupMenu&, std::uint32_t index) {
    static_cast<RageWeaponSelector&>(owner).context_selected(index);
}
void RageWeaponSelector::context_selected(std::uint32_t index) { // 0x433E3B88 / 0x433E7F67
    if (context_group >= 12)
        return;
    if (index == 0) {
        if (!actions.copy_group)
            throw std::logic_error("RAGE selector clipboard encoder is not installed");
        actions.copy_group(actions.context, context_group);
    } else if (index == 1) {
        if (!actions.paste_group)
            throw std::logic_error("RAGE selector clipboard decoder is not installed");
        if (actions.paste_group(actions.context, context_group) &&
            context_group == (value ? *value : 0)) {
            if (!actions.switch_group)
                throw std::logic_error("RAGE selector group store is not installed");
            actions.switch_group(actions.context, true);
        }
    }
}
void RageWeaponSelector::draw_checkbox(RenderQueue& queue, Point p, std::uint32_t group, i32 height,
                                       std::uint32_t accent) const { // 0x433D24EC
    const auto color = group_flag(enabled_groups, group) ? accent : 0xbb646464;
    const auto y = add(p.y, sub(height, checkbox_size) / 2);
    queue.rectangle_outline({p.x, y, checkbox_size, checkbox_size, 0xff0c0c0c}, 1);
    queue.vertical_gradient({add(p.x, 1), add(y, 1), sub(checkbox_size, 2), sub(checkbox_size, 2),
                             color, subtract_color(color, 0x00282828)});
}
void RageWeaponSelector::draw_option(RenderQueue& queue, Point p, i32 height, std::uint32_t index,
                                     std::uint32_t flags,
                                     std::uint32_t accent) const { // 0x4332C152
    if (index >= options.size())
        throw std::out_of_range("RAGE selector option is outside its vector");
    const auto group = options[index].value;
    if (group != UINT32_MAX) {
        if (group >= symbols.size())
            throw std::out_of_range("RAGE selector icon group is invalid");
        if (!icons.load)
            throw std::logic_error("RAGE selector icon cache is not installed");
        RageSelectorIcon icon;
        const auto requested = group ? icon_size.y : global_icon_size;
        if (icons.load(icons.context, queue, symbols[group], {requested, requested}, icon)) {
            const auto tint = flags & 1 || index == selected_item ? 0xc8ffffffU : 0x50ffffffU;
            const auto x = add(p.x, sub(sub(geometry.bounds.width, icon.size.x),
                                        round_layout_coordinate(scale * 40.0f)));
            const auto y = icon.size.y < height ? add(p.y, sub(height, icon.size.y) / 2) : p.y;
            queue.texture(
                {x, y, std::uint32_t(icon.texture), tint, 0, 0, icon.size.x, icon.size.y, 0, 1.0f});
        }
    }
    const Point checkbox{add(p.x, round_layout_coordinate(scale * 13.0f)), p.y};
    draw_checkbox(queue, checkbox, group, height, accent);
    const Point text{add(checkbox.x, round_layout_coordinate(scale * 20.0f)), sub(p.y, 1)};
    ComboBox::draw_option(queue, text, height, index, 0xc8ffffff,
                          index == selected_item ? flags | 1 : flags);
}
void RageWeaponSelector::draw_combo(RenderQueue& queue, Point origin, std::uint32_t accent) const {
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    draw_label(queue, position, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 8));
    const auto text_color = enabled ? 0x88ffffffU : 0x32ffffffU;
    const auto height = !name.empty() && !inline_element
                            ? sub(geometry.bounds.height, interaction_offset)
                            : geometry.bounds.height;
    draw_option(queue, {add(position.x, round_layout_coordinate(scale * 7.0f)), position.y}, height,
                selected_item, 0, accent);
    draw_down_arrow(
        queue,
        {add(sub(position.x, round_layout_coordinate(scale * 12.0f)), geometry.bounds.width),
         add(position.y, sub(height, 4) / 2)},
        text_color);
    if (!enabled || !open)
        return;
    const auto bounds_reset = queue.is_bounds_reset();
    const auto depth = queue.get_depth();
    queue.reset_bounds(false);
    queue.set_depth(0.15f);
    const Point popup{add(origin.x, geometry.bounds.x),
                      add(add(origin.y, geometry.bounds.y), add(geometry.bounds.height, 1))};
    const auto rows = std::uint32_t(std::min<std::size_t>(8, options.size()));
    queue.outlined_rectangle(
        {popup.x, popup.y, geometry.bounds.width, mul(row_height, i32(rows)), 0xff232323},
        0xff0c0c0c, 1);
    const auto offset = scroll.enabled ? scroll.position / row_height : 0;
    for (std::uint32_t row = 0; row < rows; ++row) {
        const auto index = row + std::uint32_t(offset);
        if (index >= options.size())
            break;
        const auto y = add(popup.y, mul(row_height, i32(row)));
        const auto highlight = row == std::uint32_t(hovered_item);
        if (highlight)
            queue.rectangle({add(popup.x, 1), add(y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), 0xff191919});
        draw_option(queue, {add(popup.x, round_layout_coordinate(scale * 7.0f)), y}, row_height,
                    index, highlight || index == selected_item ? 1 : 0, accent);
    }
    if (scroll.enabled)
        draw_scrollbar(queue, scroll, popup, scale);
    queue.reset_bounds(bounds_reset);
    queue.set_depth(depth);
}
void RageWeaponSelector::draw(RenderQueue& queue, Point origin,
                              std::uint32_t accent) const { // 0x4336F7D5
    draw_combo(queue, origin, accent);
    context_menu.draw(queue, origin, accent);
}
}

// ============================================================================
// Subcomponent: root_menu.cpp
// ============================================================================

namespace skeet::ui {

namespace {
void aa_notification(MenuWindow&, std::uint32_t);
struct AANotification {
    MenuWindow& window;
    WindowNotification node{0x16, aa_notification, nullptr};
    explicit AANotification(MenuWindow& owner) : window(owner) {
        if (window.notifications.size() < 2)
            throw std::logic_error("Main menu requires initialized window notifications");
        const auto bucket = node.key % (window.notifications.size() - 1);
        node.next = window.notifications[bucket];
        window.notifications[bucket] = &node;
    }
    ~AANotification() {
        if (window.notifications.size() < 2)
            return;
        auto** link = &window.notifications[node.key % (window.notifications.size() - 1)];
        while (*link && *link != window.notifications.back() && *link != &node)
            link = &(*link)->next;
        if (*link == &node)
            *link = node.next;
    }
};
struct RootRageTab final : Tab {
    RootMenuBindings& bindings;
    std::uint32_t selected = 0;
    RageWeaponGroupState group_state;
    RageWeaponClipboardContext clipboard;
    RageWeaponMenu weapon;
    RageAimbotMenu aimbot;
    RageOtherMenu other;
    RootRageTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"RAGE", next, b.identities),
          bindings(b), group_state{window,
                                   b.configuration.descriptors,
                                   b.rage_groups.blob_descriptors(),
                                   b.baseline_a,
                                   b.baseline_b,
                                   0},
          clipboard{group_state, &selected, &b.clipboard},
          weapon(*this, selected, b.rage_groups.enabled_descriptors(),
                 rage_weapon_clipboard_actions(clipboard), b.identities),
          aimbot(*this, b.configuration, b.identities),
          other(*this, b.configuration, b.identities) {
        weapon.selector().icons = b.icons.selector_provider();
    }
    void activate() override {
        update_rage_hit_chance_visibility(*aimbot.find(0x3f5e5553), bindings.engine,
                                          bindings.accuracy_variable);
    }
};
struct RootAntiAimTab final : Tab {
    AANotification notification;
    AntiAimMenu angles;
    FakeLagMenu fake_lag;
    AntiAimOtherMenu other;
    RootAntiAimTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"AA", next, b.identities),
          notification(window), angles(*this, b.configuration, b.identities),
          fake_lag(*this, b.configuration, b.identities),
          other(*this, b.configuration, b.identities) {}
};
void aa_notification(MenuWindow& window, std::uint32_t) { // 4334191F
    if (window.tabs.size() > static_cast<std::size_t>(RootTab::anti_aim))
        static_cast<RootAntiAimTab*>(window.tabs[static_cast<std::size_t>(RootTab::anti_aim)])
            ->fake_lag.limit_slider()
            .update_value_text();
}
struct RootLegitTab final : Tab {
    LegitMenu menu;
    RootLegitTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"LEGIT", next, b.identities),
          menu(*this, b.configuration, b.identities) {
        menu.weapon_selector().icons = b.icons.weapon_provider();
    }
};
struct RootVisualsTab final : Tab {
    PlayerEspMenu player;
    ColoredModelsMenu models;
    OtherEspMenu other;
    EffectsMenu effects;
    RootVisualsTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"VISUALS", next, b.identities),
          player(*this, b.configuration, b.identities),
          models(*this, b.configuration, b.identities), other(*this, b.configuration, b.identities),
          effects(*this, b.configuration, b.identities, b.camera) {}
};
struct RootMiscTab final : Tab {
    RootMenuBindings& bindings;
    MiscellaneousMenu miscellaneous;
    MovementMenu movement;
    SettingsMenu settings;
    RootMiscTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"MISC", next, b.identities),
          bindings(b), miscellaneous(*this, b.configuration, b.identities, b.miscellaneous,
                                     b.upgrade_tablet, b.expose_tablet),
          movement(*this, b.configuration, b.identities),
          settings(*this, b.configuration, b.identities, b.settings_values, b.settings_runtime) {}
    void activate() override { // 433F4A39
        auto* slider = settings.max_usercmd_ticks_slider();
        if (!slider || !slider->visible ||
            bindings.settings_values.maximum_usercmd_ticks ==
                bindings.globals.previous_max_usercmd_ticks)
            return;
        bindings.globals.previous_max_usercmd_ticks =
            bindings.settings_values.maximum_usercmd_ticks;
        slider->value_changed();
    }
};
struct RootSkinsTab final : Tab {
    SkinsMenu menu;
    RootSkinsTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"SKINS", next, b.identities),
          menu(*this, b.configuration, b.identities, b.skins_catalog, b.skins_refresh) {}
    void activate() override {
        menu.activate();
    }
};
struct RootPlayersTab final : Tab {
    PlayersMenu menu;
    RootPlayersTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"PLAYERS", next, b.identities),
          menu(*this, b.identities, b.player_state, b.player_bindings) {}
    void activate() override {
        menu.activate();
    }
};
struct RootConfigTab final : Tab {
    ConfigPresetsMenu presets;
    ScriptsMenu scripts;
    RootConfigTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"CONFIG", next, b.identities),
          presets(*this, b.identities, b.presets),
          scripts(*this, b.identities, b.script_values, b.scripts.modules().manager()) {
        presets.refresh();
    }
    void activate() override {
        scripts.activate();
    }
};
struct RootLuaTab final : Tab {
    std::unique_ptr<Category> first, second;
    RootLuaTab(MenuWindow& window, RootMenuBindings& b, Point& next)
        : Tab(window.navigation, window.input, window.geometry, u"Lua", next, b.identities) {}
    void construct_categories(WidgetIdentities& identities) {
        first =
            std::make_unique<Category>(this, u"A", GridPosition{0, 12, 0, 24}, 0, true, identities);
        categories.push_back(first.get());
        second = std::make_unique<Category>(this, u"B", GridPosition{12, 12, 0, 24}, 0, true,
                                            identities);
        categories.push_back(second.get());
    }
};
void reset_tab(void* raw) {
    static_cast<Tab*>(raw)->reset_input();
}
void activate_tab(void* raw) {
    static_cast<Tab*>(raw)->activate();
}
}

struct RootMenu::Data {
    MenuWindow& window;
    RootMenuBindings bindings;
    script::UiBindings& script_ui;
    script::ClientBindings& script_client;
    MenuWindow* window_address;
    PlayersMenu* player_menu_address = nullptr;
    MainMenuAssets assets;
    std::optional<RendererDpiBridge> renderer_dpi;
    std::array<MenuTab, 9> navigation{};
    RootRageTab* rage = nullptr;
    RootAntiAimTab* anti_aim = nullptr;
    RootLegitTab* legit = nullptr;
    RootVisualsTab* visuals = nullptr;
    RootMiscTab* misc = nullptr;
    RootSkinsTab* skins = nullptr;
    RootPlayersTab* players = nullptr;
    RootConfigTab* config = nullptr;
    RootLuaTab* lua = nullptr;
    Data(MenuWindow& w, RootMenuBindings b)
        : window(w), bindings(b), script_ui(b.scripts.modules().ui_bindings()),
          script_client(b.scripts.modules().client().bindings), window_address(&w) {}
    template <class Type> Type* add(Point& next) {
        auto tab = std::make_unique<Type>(window, bindings, next);
        auto* result = tab.get();
        append_menu_tab(window, std::move(tab));
        return result;
    }
    RootMenuLayoutTargets targets() const {
        const auto legit_categories = legit->menu.categories();
        return {{{&rage->aimbot.category(), &rage->other.category(), &anti_aim->angles.category(),
                  &anti_aim->other.category(), &anti_aim->fake_lag.category(), legit_categories[1],
                  legit_categories[2], legit_categories[3], &visuals->player.category(),
                  &visuals->models.category(), &visuals->other.category(),
                  &visuals->effects.category(), &misc->miscellaneous.category(),
                  &misc->movement.category(), &misc->settings.category(),
                  &config->presets.category(), &config->scripts.category()}},
                {{&players->menu.players_category(), &players->menu.adjustments_category()}}};
    }
};
RootMenu::RootMenu(MenuWindow& window, RootMenuBindings bindings)
    : data_(std::make_unique<Data>(window, bindings)) {
    auto& modules = bindings.scripts.modules();
    if (!bindings.baseline_a.same_source(bindings.presets.baseline_view()) ||
        !bindings.baseline_b.is_script_state(modules.shutdown_state()))
        throw std::invalid_argument(
            "Main menu requires the preset baseline and its script owner's live configuration");
    modules.validate_menu(window, bindings.configuration, bindings.identities,
                          bindings.player_state, bindings.input);
    modules.validate_renderer_binding(bindings.renderer, bindings.dpi);
    modules.bind_script_settings(bindings.script_values);
    if (!window.tabs.empty())
        throw std::invalid_argument("Main menu composition requires its initialized empty window");
    if (bindings.input.main_menu != &window)
        throw std::invalid_argument("Main menu composition requires the canonical input window");
    if ((bindings.input.hotkey_registry &&
         bindings.input.hotkey_registry != &bindings.configuration.hotkeys) ||
        (bindings.input.menu_hotkey &&
         bindings.input.menu_hotkey != &bindings.settings_values.menu_key))
        throw std::invalid_argument("Main menu and input must use the same registered menu hotkey");
    if (static_cast<const void*>(&bindings.settings_values.lock_layout) !=
        static_cast<const void*>(&window.event_state->window_size_locked))
        throw std::invalid_argument("Main menu layout lock must use the native window-state byte");
    if (&bindings.settings_values.dpi_scale != &bindings.dpi.selection)
        throw std::invalid_argument("Main menu DPI must use the shared settings value");
    bindings.input.bind_configuration(bindings.configuration, bindings.settings_values.menu_key);
    auto& d = *data_;
    d.renderer_dpi.emplace(bindings.dpi, bindings.renderer);
    bindings.settings_runtime.gui_dpi = &bindings.dpi;
    bindings.settings_runtime.main_menu = &window;
    bindings.settings_runtime.engine = bindings.engine;
    window.tabs.reserve(9);
    Point next{6, 20};
    d.rage = d.add<RootRageTab>(next);
    d.anti_aim = d.add<RootAntiAimTab>(next);
    bindings.settings_runtime.fakelag_limit = &d.anti_aim->fake_lag.limit_slider();
    d.legit = d.add<RootLegitTab>(next);
    d.visuals = d.add<RootVisualsTab>(next);
    d.misc = d.add<RootMiscTab>(next);
    d.skins = d.add<RootSkinsTab>(next);
    d.players = d.add<RootPlayersTab>(next);
    d.player_menu_address = &d.players->menu;
    d.config = d.add<RootConfigTab>(next);
    d.lua = d.add<RootLuaTab>(next);
    d.lua->construct_categories(bindings.identities);
    d.lua->set_visible(false);
    bindings.skins_catalog.initialize(d.skins->menu);
    d.lua->resize(window.input.slider_metrics);
    const auto targets = d.targets();
    constexpr std::array<std::size_t, 15> reset_indices{2,  3,  4,  5,  6,  7,  8, 9,
                                                        10, 11, 12, 13, 14, 15, 16};
    for (std::size_t i = 0; i < reset_indices.size(); ++i)
        bindings.settings_runtime.reset_targets.categories[i] =
            targets.categories[reset_indices[i]];
    bindings.settings_runtime.reset_targets.additional_categories = targets.players;
    for (std::size_t i = 0; i < d.navigation.size(); ++i)
        d.navigation[i] = {window.tabs[i], reset_tab, activate_tab};
    window.navigation.tabs = d.navigation;
    modules.bind_menu(window, bindings.configuration, bindings.identities, bindings.player_state,
                      bindings.input, d.players->menu);
}
RootMenu::~RootMenu() {
    auto& d = *data_;
    if (d.bindings.input.main_menu == d.window_address &&
        d.window.navigation.tabs.data() == d.navigation.data())
        d.window.navigation.tabs = {};
    if (d.script_ui.window == d.window_address)
        d.script_ui.window = nullptr;
    if (d.script_client.players_menu == d.player_menu_address)
        d.script_client.players_menu = nullptr;
}
Tab& RootMenu::tab(RootTab index) const {
    return *data_->window.tabs.at(static_cast<std::size_t>(index));
}
const std::uintptr_t& RootMenu::native_window() const noexcept {
    return data_->window.native_window;
}
MenuWindow& RootMenu::window() const noexcept {
    return data_->window;
}
RageWeaponMenu& RootMenu::rage_weapon_menu() const {
    return data_->rage->weapon;
}
RageWeaponGroupState& RootMenu::rage_group_state() const {
    return data_->rage->group_state;
}
std::uint32_t& RootMenu::selected_rage_group() const {
    return data_->rage->selected;
}
SkinsMenu& RootMenu::skins_menu() const {
    return data_->skins->menu;
}
LegitMenu& RootMenu::legit_menu() const {
    return data_->legit->menu;
}
PlayersMenu& RootMenu::players_menu() const {
    return data_->players->menu;
}
ConfigPresetsMenu& RootMenu::presets_menu() const {
    return data_->config->presets;
}
ConfigPresetsManager& RootMenu::preset_manager() const noexcept {
    return data_->bindings.presets;
}
ScriptsMenu& RootMenu::scripts_menu() const {
    return data_->config->scripts;
}
script::ScriptModulesOwner& RootMenu::script_modules() const noexcept {
    return data_->bindings.scripts.modules();
}
script::ScriptOwner& RootMenu::script_owner() const noexcept {
    return data_->bindings.scripts;
}
void RootMenu::initialize_renderer_resources(RendererContainer& renderer) {
    initialize_main_menu_assets(data_->window, renderer.backend(), data_->assets);
}
std::uint32_t RootMenu::tab_atlas_texture() const noexcept {
    return data_->assets.tab_atlas;
}
RootMenuLayoutTargets RootMenu::layout_targets() const {
    return data_->targets();
}
void RootMenu::capture_layout(std::array<std::int32_t, 21>& destination) {
    auto& runtime = data_->bindings.settings_runtime;
    capture_menu_layout(data_->targets(), destination, &destination == &runtime.default_layout,
                        runtime.reset_targets.additional_defaults);
}
}

// ============================================================================
// Subcomponent: root_menu_layout.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::uint32_t grid(const Category& category) noexcept {
    return std::uint8_t(category.column) |
           (std::uint32_t(std::uint8_t(category.column_span)) << 8) |
           (std::uint32_t(std::uint8_t(category.row)) << 16) |
           (std::uint32_t(std::uint8_t(category.row_span)) << 24);
}
template <class Value> Value read(const void* source, std::size_t offset) {
    Value value;
    std::memcpy(&value, static_cast<const std::byte*>(source) + offset, sizeof(value));
    return value;
}
}
Tab& append_menu_tab(MenuWindow& window, std::unique_ptr<Tab> owned) { // 0x43342275
    auto* tab = owned.get();
    tab->identity = static_cast<std::uint32_t>(window.tabs.size());
    window.tabs.push_back(tab);
    owned.release();
    tab->reflow_categories();
    tab->layout_categories();
    tab->reflow_categories();
    return *tab;
}
void capture_menu_layout(const RootMenuLayoutTargets& targets,
                         std::array<std::int32_t, 21>& destination, bool built_in,
                         std::array<std::uint32_t, 2>& player_defaults) { // 0x433ACDC7
    destination[0] = 9;
    constexpr std::array<std::size_t, 17> indices{1,  2,  3,  5,  4,  6,  7,  8, 9,
                                                  10, 11, 12, 13, 14, 15, 19, 20};
    for (std::size_t i = 0; i < indices.size(); ++i)
        destination[indices[i]] = std::bit_cast<std::int32_t>(grid(*targets.categories[i]));
    if (built_in) {
        player_defaults[0] = grid(*targets.players[0]);
        player_defaults[1] = grid(*targets.players[1]);
    }
}
void update_rage_hit_chance_visibility(Widget& hit_chance, void* engine_object,
                                       const void* accuracy_variable) { // 0x433815C4
    bool show = true;
    if (engine::invoke<bool>(engine_object, 26)) {
        auto* variable = accuracy_variable;
        for (;;) {
            const auto* parent = read<const void*>(variable, 0x1c);
            if (parent == variable)
                break;
            variable = parent;
        }
        show = read<std::uint32_t>(variable, 0x30) ==
               std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
    }
    hit_chance.set_visible(show);
}

}

// ============================================================================
// Subcomponent: scripts_menu.cpp
// ============================================================================

namespace skeet::ui {

ScriptsMenu::ScriptsMenu(Tab& tab, WidgetIdentities& identities, ScriptsMenuValues values,
                         script::ScriptManager& manager)
    : tab_(tab), category_(tab, identities), values_(values), manager_(manager) {
    storage_.reserve(8);
    bindings_.reserve(8);
    auto& enabled = add<Checkbox>(category_, 0, u"Enabled", &values_.enabled, identities);
    enabled.callbacks.push_back({this, enabled_changed});
    auto& unsafe =
        add<Checkbox>(category_, 0, u"Allow unsafe scripts", &values_.allow_unsafe, identities);
    unsafe.callbacks.push_back({this, unsafe_changed});
    add<Button>(category_, 0, u"Reload active scripts", Point{158, 28},
                WidgetCallback{this, reload_active}, identities);
    list_ = &add<Listbox>(category_, 0, u"Scripts", Point{158, 126}, nullptr, true, identities);
    list_->callbacks.push_back({this, selection_changed});
    list_->enabled = true;
    category_.scripts = list_;
    updated_ = &add<Label>(category_, 0, u" ", identities);
    startup_ = &add<Checkbox>(category_, 0, u"Load on startup", nullptr, identities);
    startup_->callbacks.push_back({this, startup_changed});
    load_ = &add<Button>(category_, 0, u"Load script", Point{158, 28}, WidgetCallback{this, load},
                         identities);
    load_->set_visible(false);
    unload_ = &add<Button>(category_, 0, u"Unload script", Point{158, 28},
                           WidgetCallback{this, unload}, identities);
    unload_->set_visible(false);
    tab_.categories.push_back(&category_);
}

ScriptsMenu::~ScriptsMenu() {
    std::erase(tab_.categories, &category_);
}

void ScriptsMenu::refresh() { // 0x4336D863
    auto discovered =
        script::discover_scripts(manager_.builtins_, manager_.files_, manager_.settings_.get());
    startup_->value = nullptr;
    startup_->config_loaded();
    std::vector<ListboxOption> options;
    // The native owner vector is replaced only for nonempty discovery. Its
    // visible list is cleared even when the previous owner records survive.
    if (!discovered.empty()) {
        manager_.records_ = std::move(discovered);
        const auto records = manager_.records();
        options.reserve(records.size());
        for (std::size_t i = 0; i < records.size(); ++i) {
            auto marker = static_cast<std::uint32_t>(i);
            if (manager_.ready() && manager_.loaded(records[i].name))
                marker |= 0x80000000u;
            options.push_back({std::bit_cast<std::int32_t>(marker), records[i].name});
        }
    }
    list_->set_options(&options);
}

void ScriptsMenu::activate() {
    if (!values_.enabled)
        return;
    refresh();
    update_selection();
}

void ScriptsMenu::update_selection() { // 0x433F1A21
    if (!values_.enabled)
        return;
    bool can_load = false, can_unload = false;
    std::uint8_t* startup = nullptr;
    const auto records = manager_.records();
    const auto index = std::bit_cast<std::uint32_t>(list_->selected_item);
    if (!list_->filtered_indices.empty() && index < records.size()) {
        auto& record = records[index];
        can_unload = manager_.ready() && manager_.loaded(record.name);
        can_load = !can_unload;
        updated_->name = script_update_age(record.modified_time, read_time(clock_context));
        startup = &record.startup;
        startup_->enabled = true;
    }
    load_->set_visible(can_load);
    unload_->set_visible(can_unload);
    startup_->value = startup;
    startup_->config_loaded();
}

void ScriptsMenu::enabled_changed(void* context, Widget& source) { // 0x43382337
    auto& self = *static_cast<ScriptsMenu*>(context);
    if (self.values_.enabled == self.values_.effective_enabled)
        return;
    self.values_.effective_enabled = self.values_.enabled;
    for (auto* control : self.category_.elements)
        if (control != &source)
            control->enabled = self.values_.enabled != 0;
    if (self.values_.enabled) {
        self.manager_.initialize();
        self.refresh();
        self.update_selection();
    } else if (self.manager_.ready()) {
        self.manager_.shutdown();
    }
}

void ScriptsMenu::unsafe_changed(void* context, Widget&) { // 0x43386459
    auto& self = *static_cast<ScriptsMenu*>(context);
    if (self.values_.allow_unsafe != self.values_.effective_unsafe) {
        self.values_.effective_unsafe = self.values_.allow_unsafe;
        self.manager_.persist_settings();
    }
}

void ScriptsMenu::reload_active(void* context, Widget&) { // 0x43359BC3
    static_cast<ScriptsMenu*>(context)->values_.reload_requested = 1;
}

void ScriptsMenu::selection_changed(void* context, Widget&) { // 0x433E1D52
    static_cast<ScriptsMenu*>(context)->update_selection();
}

void ScriptsMenu::startup_changed(void* context, Widget& source) { // 0x433A125F
    auto& self = *static_cast<ScriptsMenu*>(context);
    if (static_cast<Checkbox&>(source).value && !self.list_->filtered_indices.empty())
        self.manager_.startup_changed();
}

void ScriptsMenu::load(void* context, Widget&) { // 0x43349FD6
    auto& self = *static_cast<ScriptsMenu*>(context);
    if (self.list_->filtered_indices.empty())
        return;
    auto& option = self.list_->options[std::uint32_t(self.list_->selected_item)];
    const auto index = std::bit_cast<std::uint32_t>(option.value) & 0x7FFFFFFFu;
    const auto records = self.manager_.records();
    if (index >= records.size())
        return;
    if (self.manager_.load(index) && self.manager_.ready() &&
        self.manager_.loaded(records[index].name))
        option.value =
            std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(option.value) | 0x80000000u);
    self.update_selection();
}

void ScriptsMenu::unload(void* context, Widget&) { // 0x433D5B5F
    auto& self = *static_cast<ScriptsMenu*>(context);
    if (self.list_->filtered_indices.empty())
        return;
    auto& option = self.list_->options[std::uint32_t(self.list_->selected_item)];
    const auto index = std::bit_cast<std::uint32_t>(option.value) & 0x7FFFFFFFu;
    if (index >= self.manager_.records().size())
        return;
    self.manager_.unload(index);
    option.value = std::int32_t(std::bit_cast<std::uint32_t>(option.value) & 0x7FFFFFFFu);
    self.update_selection();
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: scripts_menu_layout.cpp
// ============================================================================

namespace skeet::ui {

std::uint64_t script_menu_time(void*) {
    return static_cast<std::uint64_t>(std::time(nullptr));
}

std::u16string script_update_age(std::uint64_t modified_time, std::uint64_t now) {
    std::uint32_t amount = 0;
    std::u16string_view unit = u"seconds";
    if (std::bit_cast<std::int64_t>(now) > std::bit_cast<std::int64_t>(modified_time)) {
        amount = static_cast<std::uint32_t>(now) - static_cast<std::uint32_t>(modified_time);
        if (amount > 60) {
            amount /= 60;
            unit = u"minutes";
            if (amount > 60) {
                amount /= 60;
                unit = u"hours";
                if (amount > 48) {
                    amount /= 24;
                    unit = u"days";
                }
            }
        }
    }
    std::array<char, 16> number{};
    const auto conversion = std::to_chars(number.data(), number.data() + number.size(), amount);
    std::u16string result = u"Updated ";
    for (auto* p = number.data(); p != conversion.ptr; ++p)
        result.push_back(char16_t(*p));
    result.push_back(u' ');
    result += unit;
    result += u" ago";
    return result;
}

ScriptsCategory::ScriptsCategory(Tab& tab, WidgetIdentities& identities)
    : Category(&tab, u"Lua", {12, 12, 0, 24}, 8, true, identities) {}

void ScriptsCategory::resize(const SliderMetrics& metrics) {
    scripts->geometry.default_size = {158, 325};
    scripts->geometry.bounds.width = 158;
    scripts->geometry.bounds.height = 325;
    scripts->dpi_changed(find_menu()->scale, metrics);
    reflow();
    Category::resize(metrics);
    scripts->update_scroll();
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: scripts_menu_startup.cpp
// ============================================================================

namespace skeet::ui {

void ScriptsMenu::initialize_startup(const char* command_line,
                                     std::span<const std::uint32_t, 128> hashes) {
    if (command_line && std::strstr(command_line, "-nolua")) { // 0x433DF55B
        static_cast<Checkbox&>(*controls()[0].widget).set_value(false);
        return;
    }
    // 0x4341AABA has no enabled-checkbox gate and is not the activation path.
    if (!hashes[0] && manager_.builtins_.empty())
        return;
    constexpr std::uint32_t mask = 0x92CA5DF1;
    if (const auto* settings = manager_.settings_.get()) {
        const auto& saved = settings->script_hashes;
        for (auto& module : manager_.builtins_)
            if (module.enabled &&
                std::find(saved.begin(), saved.end(),
                          script::script_name_hash(std::string_view(module.name)) ^ mask) !=
                    saved.end())
                module.enabled = false;
    }
    refresh();
    if (manager_.records_.empty())
        return;
    auto persist = [&](std::uint32_t excluded) {
        if (!manager_.persistence_)
            throw std::logic_error("Script startup persistence is not bound");
        (void)manager_.persistence_->persist(0, excluded);
    };
    // Keep the input as a live span: persistence may rewrite the same native
    // owner array while the startup loop is walking it.
    for (std::size_t i = 0; i < hashes.size() && hashes[i]; ++i) {
        const auto hash = hashes[i];
        auto record = std::find_if(manager_.records_.begin(), manager_.records_.end(),
                                   [&](const auto& entry) {
                                       return (script::script_name_hash(entry.name) ^ mask) == hash;
                                   });
        if (record == manager_.records_.end())
            continue;
        persist(hash);
        if ((record->name.empty() || record->name.front() != u'*') &&
            manager_.runtime_.ensure_state()) {
            const auto name = std::u16string_view(record->name).substr(0, record->name.find(u'\0'));
            if (manager_.load_module(config_name_to_utf8(name)) == 0) {
                manager_.loaded_files_.insert(std::u16string(name));
                persist(0);
                continue;
            }
        }
        if (!record->name.empty())
            record->startup = 0;
    }
    (void)manager_.runtime_.ensure_state();
    for (auto& option : list_->options) {
        const auto marker = std::bit_cast<std::uint32_t>(option.value);
        const auto index = marker & 0x7FFFFFFFu;
        if (index < manager_.records_.size() && manager_.loaded(manager_.records_[index].name))
            option.value = std::bit_cast<std::int32_t>(marker | 0x80000000u);
    }
    manager_.current_ui_config_ = manager_.save_ui_values();
    manager_.shutdown_.current_config = manager_.current_ui_config_;
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: settings_menu_storage.cpp
// ============================================================================

namespace skeet::ui {

SettingsMenuStorage::SettingsMenuStorage(WindowEventState& window) noexcept
    : menu_key(defaults_.menu_key), menu_color(defaults_.menu_color),
      dpi_scale(defaults_.dpi_scale), lock_layout(window.window_size_locked),
      multithreading_config(defaults_.multithreading_config),
      multithreading(multithreading_config.value), maximum_unlag(defaults_.maximum_unlag),
      maximum_usercmd_ticks(defaults_.maximum_usercmd_ticks),
      clock_correction(defaults_.clock_correction), hold_aim(defaults_.hold_aim),
      fix_viewmodel_lag(defaults_.fix_viewmodel_lag), fix_networking(defaults_.fix_networking),
      fix_animations(defaults_.fix_animations), faster_grenade_toss(defaults_.faster_grenade_toss) {
}

SettingsMenuStorage::SettingsMenuStorage(std::shared_ptr<engine::NativeDataImage> image)
    : image_(std::move(image)),
      menu_key(
          engine::require_native_data_image(image_).native_cell<RegisteredHotkey>(0x43478DB8u)),
      menu_color(image_->native_cell<std::uint32_t>(0x43468FB0u)),
      dpi_scale(image_->native_cell<std::int32_t>(0x43475A94u)),
      lock_layout(image_->native_cell<WindowEventState>(0x43467E4Bu).window_size_locked),
      multithreading_config(image_->native_cell<BooleanConfig>(0x434683ECu)),
      multithreading(multithreading_config.value),
      maximum_unlag(image_->native_cell<std::int32_t>(0x43468230u)),
      maximum_usercmd_ticks(image_->native_cell<std::int32_t>(0x43469274u)),
      clock_correction(image_->native_cell<std::int32_t>(0x43468290u)),
      hold_aim(image_->native_cell<std::uint8_t>(0x43468D7Cu)),
      fix_viewmodel_lag(image_->native_cell<std::uint8_t>(0x4346A328u)),
      fix_networking(image_->native_cell<std::uint8_t>(0x43467CC0u)),
      fix_animations(image_->native_cell<std::uint8_t>(0x4346A527u)),
      faster_grenade_toss(image_->native_cell<std::uint8_t>(0x43467560u)) {}

void initialize_source_settings_menu_data(engine::NativeDataImage& data) {
    WindowEventState events;
    SettingsMenuStorage defaults(events);
    data.native_cell<RegisteredHotkey>(0x43478DB8u) = {};
    data.native_cell<std::uint32_t>(0x43468FB0u) = defaults.menu_color;
    data.native_cell<std::int32_t>(0x43475A94u) = defaults.dpi_scale;
    data.native_cell<WindowEventState>(0x43467E4Bu) = events;
    auto& descriptor = data.native_cell<BooleanConfig>(0x434683ECu);
    descriptor.next = nullptr;
    descriptor.previous = nullptr;
    descriptor.hash = defaults.multithreading_config.hash;
    descriptor.serialized_size = defaults.multithreading_config.serialized_size;
    descriptor.type = defaults.multithreading_config.type;
    descriptor.reserved = defaults.multithreading_config.reserved;
    descriptor.value = defaults.multithreading;
    data.native_cell<std::int32_t>(0x43468230u) = defaults.maximum_unlag;
    data.native_cell<std::int32_t>(0x43469274u) = defaults.maximum_usercmd_ticks;
    data.native_cell<std::int32_t>(0x43468290u) = defaults.clock_correction;
    data.native_cell<std::uint8_t>(0x43468D7Cu) = defaults.hold_aim;
    data.native_cell<std::uint8_t>(0x4346A328u) = defaults.fix_viewmodel_lag;
    data.native_cell<std::uint8_t>(0x43467CC0u) = defaults.fix_networking;
    data.native_cell<std::uint8_t>(0x4346A527u) = defaults.fix_animations;
    data.native_cell<std::uint8_t>(0x43467560u) = defaults.faster_grenade_toss;
}

SettingsMenuValues SettingsMenuStorage::values() noexcept {
    return {menu_key,
            menu_color,
            dpi_scale,
            lock_layout,
            multithreading,
            maximum_unlag,
            maximum_usercmd_ticks,
            clock_correction,
            hold_aim,
            fix_viewmodel_lag,
            fix_networking,
            fix_animations,
            faster_grenade_toss};
}

} //namespace skeet::ui

// ============================================================================
// Subcomponent: skins_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement skins_required[] = {
    {0xDE88B6DC, ConfigType::boolean}, {0xF6D4D395, ConfigType::boolean},
    {0x588FC522, ConfigType::integer}, {0x82097AD3, ConfigType::integer},
    {0x3A8EF907, ConfigType::boolean}, {0xE5249CA0, ConfigType::boolean},
    {0x7439E8B5, ConfigType::integer}, {0x5101537E, ConfigType::integer},
    {0xE13F61E0, ConfigType::integer}, {0x7CFD0011, ConfigType::integer},
    {0x191A5A61, ConfigType::boolean}, {0x6CAF4832, ConfigType::boolean},
    {0xD347BC87, ConfigType::boolean}, {0xD4F2A320, ConfigType::boolean},
    {0xE0BDA969, ConfigType::integer}, {0xB3C4B6CA, ConfigType::integer},
    {0x3EB3F20B, ConfigType::integer}, {0x8191D9BC, ConfigType::integer},
    {0x9AB5333C, ConfigType::boolean}, {0xB6218FEC, ConfigType::integer_array},
};
}
SkinsMenu::SkinsMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities,
                     skins::Catalog& catalog, skins::Refresh& refresh)
    : tab_(tab), settings_(settings), catalog_(catalog), refresh_(refresh),
      models_(&tab, u"Model options", {0, 12, 0, 24}, 0, false, identities) {
    require_settings(settings, skins_required);
    if (descriptor(settings, 0xB6218FEC).serialized_size != 0x410)
        throw std::invalid_argument("SKINS requires the native 65-record configuration");
    auto boolean = [&](std::uint32_t h) { return value<BooleanConfig>(settings, h); };
    auto integer = [&](std::uint32_t h) { return value<IntegerConfig>(settings, h); };
    storage_.reserve(24);
    bindings_.reserve(24);
    tab.categories.push_back(&models_);
    team_ = &add<Slider>(models_, 0, u"Team", 2, 3, &selected_team_, true, 0, 1.0f, identities);
    team_->set_visible(false);
    team_->callbacks.push_back({this, team_changed}); // 0x433EB2C8
    knife_ = &add<Checkbox>(models_, 0xD347BC87, u"Knife changer", boolean(0xD347BC87), identities);
    knife_->callbacks.push_back({this, knife_enabled_changed});
    knife_model_ = &add<ComboBox>(models_, 0xE0BDA969, u"", integer(0xE0BDA969), false, identities);
    knife_model_->set_options({{u"Bayonet", 0},
                               {u"Flip", 1},
                               {u"Gut", 2},
                               {u"Karambit", 3},
                               {u"M9 Bayonet", 4},
                               {u"Tactical", 5},
                               {u"Butterfly", 6},
                               {u"Falchion", 7},
                               {u"Shadow dagger", 8},
                               {u"Survival bowie", 9},
                               {u"Ursus", 10},
                               {u"Navaja", 11},
                               {u"Stiletto", 12},
                               {u"Talon", 13},
                               {u"Classic knife", 14},
                               {u"Paracord", 15},
                               {u"Survival", 16},
                               {u"Nomad", 17},
                               {u"Skeleton", 18}});
    knife_model_->callbacks.push_back({this, knife_model_changed});
    knife_model_->set_visible(false);
    dependencies_[0] = {knife_model_, &knife_->tied};
    knife_->tied.next = &dependencies_[0];
    glove_ = &add<Checkbox>(models_, 0x3A8EF907, u"Glove changer", boolean(0x3A8EF907), identities);
    glove_->callbacks.push_back({this, glove_enabled_changed}); // 0x433D66B5
    glove_model_ = &add<ComboBox>(models_, 0x7439E8B5, u"", integer(0x7439E8B5), false, identities);
    glove_model_->callbacks.push_back({this, glove_model_changed}); // 0x4333CEA9
    glove_paint_ = &add<ComboBox>(models_, 0xE13F61E0, u"", integer(0xE13F61E0), false, identities);
    // 0x433CCE8E is an actual empty callback; no replacement work is attached.
    // The model and paint choices are supplied by the live item schema.
    glove_model_->set_visible(false);
    glove_paint_->set_visible(false);
    dependencies_[1] = {glove_paint_, &dependencies_[2]};
    dependencies_[2] = {glove_model_, &glove_->tied};
    glove_->tied.next = &dependencies_[1];
    mask_ = &add<Checkbox>(models_, 0xDE88B6DC, u"Mask changer", boolean(0xDE88B6DC), identities);
    mask_model_ = &add<ComboBox>(models_, 0x588FC522, u"", integer(0x588FC522), false, identities);
    mask_model_->set_options({{u"Hoxton", 0},     {u"Doll", 1},     {u"Samurai", 3},
                              {u"Clown", 4},      {u"Wolf", 5},     {u"Sheep", 6},
                              {u"Gold bunny", 7}, {u"Anaglyph", 8}, {u"Japanese", 9},
                              {u"Dallas", 10},    {u"Pumpkin", 11}, {u"Bloody sheep", 12},
                              {u"Devil", 13},     {u"Boar", 14},    {u"Chains", 15},
                              {u"Tiki", 16},      {u"Bunny", 17},   {u"Gold sheep", 18},
                              {u"Zombie", 19},    {u"Chicken", 20}, {u"Gold skull", 21}});
    mask_model_->set_visible(false);
    dependencies_[3] = {mask_model_, &mask_->tied};
    mask_->tied.next = &dependencies_[3];
    agent_ = &add<Checkbox>(models_, 0x191A5A61, u"Agent changer", boolean(0x191A5A61), identities);
    agent_->callbacks.push_back({this, agent_enabled_changed});
    agent_t_ = &add<ComboBox>(models_, 0x3EB3F20B, u"", integer(0x3EB3F20B), false, identities);
    agent_ct_ = &add<ComboBox>(models_, 0x8191D9BC, u"", integer(0x8191D9BC), false, identities);
    agent_t_->set_visible(false);
    agent_ct_->set_visible(false);
    weapon_ = std::make_unique<Category>(&tab, u"Weapon skin", GridPosition{12, 12, 0, 24}, 14,
                                         false, identities);
    tab.categories.push_back(weapon_.get());
    weapon_selector_ =
        &add<Slider>(*weapon_, 0, u"Weapon", 0, 1000, &selected_weapon_, true, 0, 1.0f, identities);
    weapon_selector_->set_visible(false);
    weapon_selector_->callbacks.push_back({this, weapon_changed});
    auto& record =
        static_cast<skins::WeaponSkinsConfig&>(descriptor(settings, 0xB6218FEC)).value[0];
    enabled_ = &add<Checkbox>(*weapon_, 0xB6218FEC, u"Enabled", &record.enabled, identities);
    enabled_->callbacks.push_back({this, enabled_changed});
    stattrak_ = &add<Checkbox>(*weapon_, 0xB6218FEC, u"StatTrak", &record.stattrak, identities);
    quality_ = &add<Slider>(*weapon_, 0xB6218FEC, u"Quality", 0, 100, &record.quality, true, 0x25,
                            1.0f, identities);
    seed_ = &add<Slider>(*weapon_, 0xB6218FEC, u"Seed", 0, 1000, &record.seed, true, 0, 1.0f,
                         identities);
    filter_ =
        &add<Checkbox>(*weapon_, 0x9AB5333C, u"Filter by weapon", boolean(0x9AB5333C), identities);
    filter_->callbacks.push_back({this, filter_changed});
    skin_ = &add<Listbox>(*weapon_, 0xB6218FEC, u"Skin", Point{158, 322}, &record.paintkit, true,
                          identities);
    skin_->callbacks.push_back({this, paint_changed});
}
SkinsMenu::~SkinsMenu() {
    for (auto* category : categories())
        std::erase(tab_.categories, category);
}
void SkinsMenu::refresh() {
    refresh_.invalidate_weapon_models();
    refresh_.schedule_full_update(refresh_.current_time() + 0.25f);
}
void SkinsMenu::select_weapon(std::int32_t item, std::int32_t team) {
    selected_weapon_ = item;
    const auto slot = skins::weapon_config_slot(item, team);
    if (slot < 0)
        return;
    auto& r = static_cast<skins::WeaponSkinsConfig&>(descriptor(settings_, 0xB6218FEC)).value[slot];
    enabled_->value = &r.enabled;
    enabled_->config_loaded();
    stattrak_->value = &r.stattrak;
    stattrak_->config_loaded();
    stattrak_->enabled = true;
    quality_->value = &r.quality;
    quality_->config_loaded();
    seed_->value = &r.seed;
    seed_->config_loaded();
    skin_->value = &r.paintkit;
    skin_->config_loaded();
    rebuild_skin_list();
}
void SkinsMenu::select_team(std::int32_t team) { // 0x4334A06E
    selected_team_ = team;
    auto previous_paint = UINT32_MAX;
    if (team == 2 || team == 3) {
        const bool ct = team == 3;
        previous_paint = *reinterpret_cast<std::uint32_t*>(
            value<IntegerConfig>(settings_, ct ? 0x7CFD0011 : 0xE13F61E0));
        mask_->value = value<BooleanConfig>(settings_, ct ? 0xF6D4D395 : 0xDE88B6DC);
        mask_->config_loaded();
        mask_model_->value = reinterpret_cast<std::uint32_t*>(
            value<IntegerConfig>(settings_, ct ? 0x82097AD3 : 0x588FC522));
        mask_model_->config_loaded();
        glove_->value = value<BooleanConfig>(settings_, ct ? 0xE5249CA0 : 0x3A8EF907);
        glove_->config_loaded();
        glove_paint_->value = reinterpret_cast<std::uint32_t*>(
            value<IntegerConfig>(settings_, ct ? 0x7CFD0011 : 0xE13F61E0));
        glove_paint_->config_loaded();
        glove_model_->value = reinterpret_cast<std::uint32_t*>(
            value<IntegerConfig>(settings_, ct ? 0x5101537E : 0x7439E8B5));
        glove_model_->config_loaded();
        knife_->value = value<BooleanConfig>(settings_, ct ? 0xD4F2A320 : 0xD347BC87);
        knife_->config_loaded();
        knife_model_->value = reinterpret_cast<std::uint32_t*>(
            value<IntegerConfig>(settings_, ct ? 0xB3C4B6CA : 0xE0BDA969));
        knife_model_->config_loaded();
        agent_->value = value<BooleanConfig>(settings_, ct ? 0x6CAF4832 : 0x191A5A61);
        agent_->config_loaded();
    }
    knife_state_ = knife_->value ? *knife_->value : 0;
    glove_state_ = glove_->value ? *glove_->value : 0;
    mask_state_ = mask_->value ? *mask_->value : 0;
    set_agent_team(team);
    const auto model = glove_model_->value ? *glove_model_->value : 0;
    rebuild_glove_paint_list(std::uint16_t(model), previous_paint);
}
void SkinsMenu::initialize_schema_options() {
    auto copy = [](std::span<const skins::CatalogChoice> source) {
        std::vector<ComboBoxOption> out;
        out.reserve(source.size());
        for (const auto& x : source)
            out.push_back({x.name, x.value});
        return out;
    };
    glove_model_->set_options(copy(catalog_.glove_models()));
    agent_t_->set_options(copy(catalog_.agent_models(2)));
    agent_ct_->set_options(copy(catalog_.agent_models(3)));
}
void SkinsMenu::rebuild_glove_paint_list() { // 0x43368201
    const auto model = glove_model_->value ? *glove_model_->value : 0;
    const auto previous = glove_paint_->value ? *glove_paint_->value : 0;
    rebuild_glove_paint_list(std::uint16_t(model), previous);
}
void SkinsMenu::rebuild_glove_paint_list(std::uint16_t model, std::uint32_t previous) {
    if (!glove_paint_)
        return;
    if (previous == UINT32_MAX)
        previous = glove_paint_->value ? *glove_paint_->value : 0;
    if (!catalog_.available())
        return;
    std::vector<ComboBoxOption> options;
    bool found = false;
    for (auto id : catalog_.paintkits(model)) {
        if (id == previous)
            found = true;
        if (auto name = catalog_.paintkit_name(id))
            options.push_back({std::move(*name), id});
    }
    skins::sort_catalog_choices(std::span<ComboBoxOption>(options));
    const auto selection = found ? previous : (options.empty() ? previous : options.front().value);
    glove_paint_->set_options(std::move(options));
    skins::set_combo_value_atomic(*glove_paint_, selection); // 43398ECF follows list replacement.
}
void SkinsMenu::activate() { // 0x4331FB95
    set_agent_team(refresh_.agent_team());
    if (skin_->options.empty() && glove_paint_->options.empty())
        initialize_default_paints();
}
void SkinsMenu::initialize_default_paints() { // 0x433D7108
    const auto previous = glove_paint_->value ? *glove_paint_->value : 0;
    rebuild_glove_paint_list(0x13A3, previous);
    rebuild_skin_list();
    if (!skin_->options.empty()) {
        const auto first = skin_->options.front().value;
        for (auto& record :
             static_cast<skins::WeaponSkinsConfig&>(descriptor(settings_, 0xB6218FEC)).value)
            record.paintkit = first;
    }
}
void SkinsMenu::set_agent_team(std::int32_t team) { // 0x433AB7C4
    const bool active = agent_->value && *agent_->value;
    if (team == 3) {
        agent_t_->set_visible(false);
        agent_ct_->set_visible(active);
    } else {
        agent_ct_->set_visible(false);
        agent_t_->set_visible(team == 2 && active);
    }
}
void SkinsMenu::rebuild_skin_list() { // 0x434084F0
    if (!skin_ || !catalog_.available())
        return;
    std::vector<ListboxOption> entries;
    auto add = [&](std::uint32_t id) {
        if (auto name = catalog_.paintkit_name(id))
            entries.push_back({std::int32_t(id), std::move(*name)});
    };
    if (filter_->value && *filter_->value) {
        for (auto id : catalog_.paintkits(std::uint16_t(selected_weapon_)))
            add(id);
    } else {
        const auto max = catalog_.maximum_paintkit();
        for (std::uint32_t id = 0; id <= max; ++id) {
            add(id);
            if (id == UINT32_MAX)
                break;
        }
    }
    skins::sort_catalog_choices(std::span<ListboxOption>(entries));
    skin_->set_options(&entries);
    skin_->config_loaded();
}
void SkinsMenu::team_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    s.select_team(s.selected_team_);
}
void SkinsMenu::weapon_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    s.select_weapon(s.selected_weapon_);
}
void SkinsMenu::filter_changed(void* p, Widget&) {
    static_cast<SkinsMenu*>(p)->rebuild_skin_list();
}
void SkinsMenu::enabled_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    auto& a = static_cast<skins::WeaponSkinsConfig&>(descriptor(s.settings_, 0xB6218FEC)).value;
    if (skins::synchronize_enabled(a, s.enabled_bits_))
        s.refresh();
}
void SkinsMenu::paint_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    if (s.enabled_->value && *s.enabled_->value)
        s.refresh();
}
void SkinsMenu::knife_enabled_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    const auto state = s.knife_->value ? *s.knife_->value : std::uint8_t(0);
    if (state != s.knife_state_) {
        s.knife_state_ = state;
        s.refresh();
    }
}
void SkinsMenu::knife_model_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    if (s.knife_->value && *s.knife_->value)
        s.refresh();
}
void SkinsMenu::agent_enabled_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    s.set_agent_team(s.refresh_.agent_team());
}
void SkinsMenu::glove_model_changed(void* p, Widget&) {
    static_cast<SkinsMenu*>(p)->rebuild_glove_paint_list();
}
void SkinsMenu::glove_enabled_changed(void* p, Widget&) {
    auto& s = *static_cast<SkinsMenu*>(p);
    s.glove_state_ = s.glove_->value ? *s.glove_->value : 0;
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: startup_config_path.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

extern "C" ULONG NTAPI RtlGetCurrentDirectory_U(ULONG, PWSTR);
extern "C" BOOLEAN NTAPI RtlDoesFileExists_U(PCWSTR);
extern "C" BOOLEAN NTAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PCWSTR*, PVOID);

namespace skeet::ui {
MainMenuConfigPathInitialization initialize_main_menu_config_path(PreservedConfigStorage& storage) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t));
    static_assert(sizeof(NativeConfigPath) == sizeof(UNICODE_STRING));
    auto& output = storage.path();
    if (output.length || output.maximum_length || output.buffer)
        throw std::logic_error("Main menu configuration path is already initialized");
    const auto cpu = read_main_menu_cpu_leaf1();
    const auto key = script::database_path_key(cpu.eax);
    // The actual startup4349CE62 call requests104h bytes into a cleared208h
    // buffer. Its return value is ignored before constructing the filename.
    std::array<wchar_t, 260> directory{};
    (void)RtlGetCurrentDirectory_U(0x104, directory.data());
    std::wstring path(directory.data());
    path += L"\\csgo\\cfg\\";
    path += std::to_wstring(key);
    path += L"9.cfg";
    const bool existed = RtlDoesFileExists_U(path.c_str()) != 0;
    UNICODE_STRING name{};
    const bool converted = RtlDosPathNameToNtPathName_U(path.c_str(), &name, nullptr, nullptr) != 0;
    // Transfer only the real API result, including its exact failure output.
    // The source's existing canonical-path finalizer retains sole ownership.
    output = {name.Length, name.MaximumLength, reinterpret_cast<char16_t*>(name.Buffer)};
    return {cpu.eax, key, existed, converted};
}
}

// ============================================================================
// Subcomponent: startup_data.cpp
// ============================================================================

namespace skeet::ui {
namespace {
std::span<const std::byte> vector_bytes(const NativeConfigVector& vector) {
    if (!vector.begin) {
        if (vector.end || vector.capacity)
            throw std::logic_error("Menu baseline has no allocation for its published range");
        return {};
    }
    if (vector.end < vector.begin || vector.capacity < vector.end)
        throw std::logic_error("Menu baseline has an invalid native vector range");
    return {reinterpret_cast<const std::byte*>(std::uintptr_t(vector.begin)),
            std::size_t(vector.end - vector.begin)};
}
}

MenuStartupData::MenuStartupData(std::shared_ptr<engine::NativeDataImage> image)
    : image_(std::move(image)),
      dpi(engine::require_native_data_image(image_).native_cell<std::int32_t>(0x43475A94u)) {}

SettingsMenuValues MenuStartupData::settings_values() const {
    auto& data = *image_;
    return {data.native_cell<RegisteredHotkey>(0x43478DB8u),
            data.native_cell<std::uint32_t>(0x43468FB0u),
            data.native_cell<std::int32_t>(0x43475A94u),
            window_events().window_size_locked,
            data.native_cell<std::uint8_t>(0x434683FCu),
            data.native_cell<std::int32_t>(0x43468230u),
            data.native_cell<std::int32_t>(0x43469274u),
            data.native_cell<std::int32_t>(0x43468290u),
            data.native_cell<std::uint8_t>(0x43468D7Cu),
            data.native_cell<std::uint8_t>(0x4346A328u),
            data.native_cell<std::uint8_t>(0x43467CC0u),
            data.native_cell<std::uint8_t>(0x4346A527u),
            data.native_cell<std::uint8_t>(0x43467560u)};
}
ScriptsMenuValues MenuStartupData::script_values() const {
    auto& data = *image_;
    return {
        data.native_cell<std::uint8_t>(0x434675D4u), data.native_cell<std::uint8_t>(0x4346D544u),
        data.native_cell<std::uint8_t>(0x43475798u), data.native_cell<std::uint8_t>(0x43478C6Eu),
        data.native_cell<std::uint8_t>(0x4347CA28u)};
}
WindowEventState& MenuStartupData::window_events() const {
    static_assert(sizeof(WindowEventState) == 1);
    return image_->native_cell<WindowEventState>(0x43467E4Bu);
}
RootMenuGlobals& MenuStartupData::root_globals() const {
    static_assert(sizeof(RootMenuGlobals) == 4);
    return image_->native_cell<RootMenuGlobals>(0x4347CA3Cu);
}
std::span<const std::byte> MenuStartupData::baseline_a() const {
    return vector_bytes(image_->native_cell<NativeConfigVector>(0x43478CB8u));
}
std::span<const std::byte> MenuStartupData::baseline_b() const {
    return vector_bytes(image_->native_cell<NativeConfigVector>(0x43479770u));
}
} //namespace skeet::ui

// ============================================================================
// Subcomponent: startup_registry.cpp
// ============================================================================
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::ui {
namespace {
#if defined(_WIN32)
template <class Function> Function imported(const char* name) {
    const auto module = GetModuleHandleA("ntdll.dll");
    const auto address = module ? GetProcAddress(module, name) : nullptr;
    if (!address)
        throw std::runtime_error("A required startup registry export is missing");
    return reinterpret_cast<Function>(address);
}
#endif
template <class T> void unlink(T& link) {
    link.previous->next = link.next;
    link.next->previous = link.previous;
}
}
StartupRegistryImports native_startup_registry_imports() {
#if defined(_WIN32)
    StartupRegistryImports imports{};
#define REGISTRY_IMPORT(member, name) imports.member = imported<decltype(imports.member)>(name)
    REGISTRY_IMPORT(current_user_path, "RtlFormatCurrentUserKeyPath");
    REGISTRY_IMPORT(append_string, "RtlAppendUnicodeStringToString");
    REGISTRY_IMPORT(free_string, "RtlFreeUnicodeString");
    REGISTRY_IMPORT(append_text, "RtlAppendUnicodeToString");
    REGISTRY_IMPORT(open_key, "NtOpenKey");
    REGISTRY_IMPORT(initialize_string, "RtlInitUnicodeString");
    REGISTRY_IMPORT(query_value, "NtQueryValueKey");
    REGISTRY_IMPORT(close, "NtClose");
#undef REGISTRY_IMPORT
    return imports;
#else
    throw std::runtime_error("Startup registry input requires Windows");
#endif
}
std::optional<ConfigBytes> read_startup_registry_value(const char16_t* name,
                                                       StartupRegistryImports imports,
                                                       engine::JobMemory memory) {
    if (!name || !imports.current_user_path || !imports.append_string || !imports.free_string ||
        !imports.append_text || !imports.open_key || !imports.initialize_string ||
        !imports.query_value || !imports.close || !memory.allocate || !memory.free ||
        !memory.process_heap)
        throw std::invalid_argument(
            "Startup registry input requires complete native imports and process heap");
    NativeConfigPath current{};
    (void)imports.current_user_path(&current);
    std::array<char16_t, 0x106> text;
    NativeConfigPath path{0, 0x20C, text.data()};
    // The supplied binary replaces its first status tests with unconditional
    // branches. Preserve the observed supported path and ignored statuses.
    (void)imports.append_string(&path, &current);
    imports.free_string(&current);
    (void)imports.append_text(&path, u"\\Software");
    StartupRegistryObjectAttributes attributes{
        sizeof(StartupRegistryObjectAttributes), nullptr, &path, 0x40, nullptr, nullptr};
    void* key = nullptr;
    (void)imports.open_key(&key, 0x20019, &attributes);
    imports.initialize_string(&path, name);
    std::uint32_t length = 0;
    (void)imports.query_value(key, &path, 2, nullptr, 0, &length);
    if (!length) {
        (void)imports.close(key);
        return std::nullopt;
    }
    auto* allocation = static_cast<std::byte*>(engine::allocate_owner_storage(memory, length));
    // Native43490D97 skips its close-key block on allocation failure.
    if (!allocation)
        return std::nullopt;
    const auto size = length;
    const auto status = imports.query_value(key, &path, 2, allocation, length, &length);
    if (status) {
        engine::release_owner_storage(memory, allocation);
        (void)imports.close(key);
        return std::nullopt;
    }
    ConfigBytes result(allocation, allocation + size);
    engine::release_owner_storage(memory, allocation);
    (void)imports.close(key);
    return result;
}
void unlink_startup_settings_records(SettingsPersistenceState& state) {
    auto* const owner = const_cast<std::byte*>(state.native_owner());
    if (!owner)
        throw std::invalid_argument(
            "Startup settings unlink requires its constructed native owner");
    constexpr std::array<std::size_t, 10> offsets{0,    0x14, 0x3C,  0x50,  0x64,
                                                  0x78, 0x8C, 0x29C, 0x2B0, 0x314};
    unlink(*reinterpret_cast<ConfigLink*>(owner));
    unlink(*reinterpret_cast<ConfigLink*>(owner + 0x14));
    unlink(*reinterpret_cast<HotkeyLink*>(owner + 0x24));
    for (std::size_t index = 2; index < offsets.size(); ++index)
        unlink(*reinterpret_cast<ConfigLink*>(owner + offsets[index]));
}
std::array<bool, 10> restore_startup_settings_records(SettingsPersistenceState& state,
                                                      std::span<const std::byte> information) {
    const auto word = [](std::span<const std::byte> bytes, std::size_t offset) {
        std::uint32_t value;
        std::memcpy(&value, bytes.data() + offset, sizeof value);
        return value;
    };
    if (information.size() < 12 || word(information, 4) != 3)
        throw std::invalid_argument("Startup settings require a native REG_BINARY value");
    const auto length = word(information, 8);
    if (length > information.size() - 12 || length < 12)
        throw std::invalid_argument("Startup settings registry payload has an invalid extent");
    const auto stream = information.subspan(12, length);
    if (word(stream, 0) != config_stream_magic)
        throw std::invalid_argument("Startup settings stream has an invalid signature");
    auto* const owner = const_cast<std::byte*>(state.native_owner());
    if (!owner)
        throw std::invalid_argument(
            "Startup settings restore requires its constructed native owner");
    constexpr std::array<std::size_t, 10> offsets{0,    0x14, 0x3C,  0x50,  0x64,
                                                  0x78, 0x8C, 0x29C, 0x2B0, 0x314};
    std::array<bool, 10> records{};
    for (std::size_t index = 0; index < offsets.size(); ++index) {
        records[index] = find_config_record(stream, state.record_keys[index]).has_value();
        if (records[index])
            (void)deserialize_config_value(
                *reinterpret_cast<ConfigDescriptor*>(owner + offsets[index]), stream, false);
    }
    return records;
}
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(StartupRegistryObjectAttributes) == 24);
#endif
} //namespace skeet::ui
