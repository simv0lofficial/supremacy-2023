#include "skeet/script/ui_module.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <cwchar>
#include <mutex>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace {
// As in the embedded library, all UI closures use one bound Lua 5.1 C API.
// Their per-state owner is retained by Lua as the closure's first upvalue.
void* (*export_context)(void*, int) = nullptr;
constexpr int globals = -10002;
constexpr int first_upvalue = -10003;

std::uint32_t name_hash(const char* value) noexcept {
    std::uint32_t hash = 0x811c9dc5;
    for (const auto* byte = reinterpret_cast<const unsigned char*>(value); *byte; ++byte)
        hash = (hash ^ *byte) * 0x01000193u;
    return hash ^ 0x92ca5df1u;
}
bool convert_argument(const char* value, std::array<char16_t, 128>& text) {
    if (!value || !*value || std::strlen(value) >= 128)
        return false;
    text.fill(0);
    return MultiByteToWideChar(CP_UTF8, 0, value, -1, reinterpret_cast<wchar_t*>(text.data()),
                               static_cast<int>(text.size())) > 1;
}
bool equal_menu_name(std::u16string_view name, const char16_t* requested) {
    // 433FAB07 -> 43418122 clears the complete 64-unit destination first.
    // An oversized encoded name leaves that buffer empty; it is never cut
    // down to a prefix that could match a different requested menu name.
    std::array<wchar_t, 64> text{};
    if (name.size() < text.size() && !name.empty())
        std::memcpy(text.data(), name.data(), name.size() * sizeof(char16_t));
    return _wcsicmp(text.data(), reinterpret_cast<const wchar_t*>(requested)) == 0;
}
std::wstring convert_label(const char* value) {
    const auto length = MultiByteToWideChar(CP_UTF8, 0, value, -1, nullptr, 0);
    if (length <= 1)
        return {};
    std::wstring text(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value, -1, text.data(), length);
    text.pop_back();
    return text;
}
}

int UiModule::parse_target(void* state, Target& target) { // 0x4341E5EF
    const auto& api = *bridge_.api;
    std::array<char, 128> key{};
    std::size_t key_length = 0;
    auto append_argument = [&](const char* value) {
        if (!value || !*value)
            return false;
        const auto length = std::strlen(value);
        if (length >= key.size() - key_length)
            return false;
        std::memcpy(key.data() + key_length, value, length + 1);
        key_length += length;
        return convert_argument(value, target.label);
    };
    const char* value = api.checklstring(state, 1, nullptr);
    if (!append_argument(value))
        return api.argerror(state, 1, "invalid argument");
    if (!bindings_.window)
        throw std::logic_error("Lua UI requires its menu window");
    for (auto* tab : bindings_.window->tabs)
        if (equal_menu_name(tab->name, target.label.data())) {
            target.tab = tab;
            break;
        }
    if (!target.tab)
        return api.argerror(state, 1, "invalid argument");
    value = api.checklstring(state, 2, nullptr);
    if (!append_argument(value))
        return api.argerror(state, 2, "invalid argument");
    for (auto* category : target.tab->categories)
        if (equal_menu_name(category->name, target.label.data())) {
            target.category = category;
            break;
        }
    if (!target.category)
        return api.argerror(state, 2, "invalid argument");
    value = api.checklstring(state, 3, nullptr);
    if (!append_argument(value) || callback_hash(key.data()) == 0xeb718867u)
        return api.argerror(state, 3, "invalid argument");
    target.hash = name_hash(key.data());
    if (target.label[0] == u'\n')
        target.label[0] = 0;
    return 0;
}

void UiModule::attach(Target& target, ui::Widget& widget) { // 0x4336C436
    if (auto* tab = ui::owning_tab(target.category))
        tab->set_visible(true);
    auto* input = widget.find_menu();
    if (!input)
        throw std::logic_error("Lua UI requires its menu input");
    std::unique_lock lock(target.category->elements_mutex);
    target.category->elements.push_back(&widget);
    widget.dpi_changed(input->scale, input->slider_metrics);
    widget.resize(input->slider_metrics);
}

int UiModule::finish(void* state, Target& target, ui::Widget& widget) {
    bridge_.api->pushinteger(state, static_cast<std::ptrdiff_t>(bindings_.records.size() - 1));
    target.tab->reflow_categories();
    widget.resize(widget.find_menu()->slider_metrics);
    return 1;
}

int UiModule::new_checkbox(void* state) { // 0x433C905F
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 3)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    auto configuration = std::make_unique<BooleanConfig>();
    initialize_boolean_config(configuration_.descriptors, *configuration, target.hash);
    auto widget =
        std::make_unique<ui::Checkbox>(target.category, std::u16string_view(target.label.data()),
                                       &configuration->value, identities_);
    auto* checkbox = widget.get();
    const auto handle = bindings_.adopt(std::move(widget), std::move(configuration));
    attach(target, *checkbox);
    api.pushinteger(state, handle);
    target.tab->reflow_categories();
    checkbox->resize(checkbox->find_menu()->slider_metrics);
    return 1;
}

int UiModule::new_slider(void* state) { // 0x4341A695
    const auto& api = *bridge_.api;
    const auto arguments = api.gettop(state);
    if (arguments < 5)
        return api.lerror(state, "invalid number of arguments");
    const auto minimum = static_cast<std::int32_t>(api.tointeger(state, 4));
    const auto maximum = static_cast<std::int32_t>(api.tointeger(state, 5));
    if (maximum <= minimum)
        return api.argerror(state, 5, "invalid slider limits");
    auto initial = minimum;
    if (arguments > 5) {
        const auto requested = static_cast<std::int32_t>(api.tointeger(state, 6));
        if (requested >= minimum)
            initial = (std::min)(requested, maximum);
    }
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    bool show = true;
    std::uint32_t suffix = 0;
    float multiplier = 1.0f;
    if (arguments >= 7)
        show = api.toboolean(state, 7) != 0;
    if (arguments >= 8 && api.type(state, 8) != 0) {
        const auto* unit = api.checklstring(state, 8, nullptr);
        std::array<char16_t, 8> text{};
        if (unit &&
            MultiByteToWideChar(CP_UTF8, 0, unit, -1, reinterpret_cast<wchar_t*>(text.data()),
                                static_cast<int>(text.size())) > 1 &&
            text[0] != u'\n' && text[1] != u'\n')
            suffix = std::uint32_t(text[0]) | (std::uint32_t(text[1]) << 16);
    }
    if (arguments >= 9)
        multiplier = static_cast<float>(api.checknumber(state, 9));
    auto configuration = std::make_unique<IntegerConfig>();
    initialize_integer_config(configuration_.descriptors, *configuration, target.hash, 0);
    auto widget = std::make_unique<ui::Slider>(
        target.category, std::u16string_view(target.label.data()), minimum, maximum,
        &configuration->value, show, suffix, multiplier, identities_);
    auto* slider = widget.get();
    const auto handle = bindings_.adopt(std::move(widget), std::move(configuration));
    attach(target, *slider);
    slider->set_value(initial);
    api.pushinteger(state, handle);
    target.tab->reflow_categories();
    slider->resize(slider->find_menu()->slider_metrics);
    if (arguments >= 10 && api.type(state, 10) == 5) {
        api.pushvalue(state, 10);
        api.pushnil(state);
        while (api.next(state, -2)) {
            api.pushvalue(state, -2);
            api.pushvalue(state, -2);
            if (api.type(state, -1) == 4 && api.type(state, -2) == 3) {
                const auto* text = api.tolstring(state, -1, nullptr);
                const auto value = static_cast<std::int32_t>(api.checkinteger(state, -2));
                if (text && *text && minimum <= value && value <= maximum) {
                    auto label = convert_label(text);
                    if (!label.empty()) {
                        slider->value_labels[std::uint32_t(value)] = std::move(label);
                        if (*slider->value == value)
                            slider->update_value_text();
                    }
                }
            }
            api.settop(state, -4);
        }
        api.settop(state, -2);
    }
    return 1;
}

int UiModule::checkbox_entry(void* state) {
    return from_state(state).new_checkbox(state);
}
int UiModule::slider_entry(void* state) {
    return from_state(state).new_slider(state);
}
UiModule& UiModule::from_state(void* state) {
    return *static_cast<UiModule*>(export_context(state, first_upvalue));
}
int UiModule::combobox_entry(void* state) {
    return from_state(state).new_combobox(state);
}
int UiModule::multiselect_entry(void* state) {
    return from_state(state).new_multiselect(state);
}
int UiModule::hotkey_entry(void* state) {
    return from_state(state).new_hotkey(state);
}
int UiModule::button_entry(void* state) {
    return from_state(state).new_button(state);
}
int UiModule::color_picker_entry(void* state) {
    return from_state(state).new_color_picker(state);
}
int UiModule::textbox_entry(void* state) {
    return from_state(state).new_textbox(state);
}
int UiModule::listbox_entry(void* state) {
    return from_state(state).new_listbox(state);
}
int UiModule::string_entry(void* state) {
    return from_state(state).new_string(state);
}
int UiModule::label_entry(void* state) {
    return from_state(state).new_label(state);
}
int UiModule::reference_entry(void* state) {
    return from_state(state).reference(state);
}
void UiModule::register_exports() {
    if (!bridge_.api || !bridge_.state)
        throw std::logic_error("Lua UI exports require an initialized state");
    bindings_.initialize(bridge_);
    const auto& api = *bridge_.api;
    export_context = api.touserdata;
    api.createtable(bridge_.state, 0, 0);
    for (const auto& function : std::array<std::pair<const char*, LuaApi::CFunction>, 12>{
             {{"new_checkbox", checkbox_entry},
              {"new_slider", slider_entry},
              {"new_combobox", combobox_entry},
              {"new_multiselect", multiselect_entry},
              {"new_hotkey", hotkey_entry},
              {"new_button", button_entry},
              {"new_color_picker", color_picker_entry},
              {"new_textbox", textbox_entry},
              {"new_listbox", listbox_entry},
              {"new_string", string_entry},
              {"new_label", label_entry},
              {"reference", reference_entry}}}) {
        api.pushlightuserdata(bridge_.state, this);
        api.pushcclosure(bridge_.state, function.second, 1);
        api.setfield(bridge_.state, -2, function.first);
    }
    values_.register_into(-1);
    api.setfield(bridge_.state, globals, "ui");
}

} // namespace skeet::script
