#include "skeet/script/ui_values.hpp"
#include "skeet/script/ui_type_storage.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/combobox.hpp"
#include "skeet/color_picker.hpp"
#include "skeet/hotkey_widget.hpp"
#include "skeet/listbox.hpp"
#include "skeet/multiselect.hpp"
#include "skeet/textbox.hpp"
#include "ui_values_detail.hpp"
#include <array>
#include <stdexcept>
#include <utility>

namespace skeet::script {
namespace {
using namespace ui_values_detail;
constexpr int first_upvalue = -10003;
void* (*closure_context)(void*, int) = nullptr;
UiValues& owner(void* state) {
    return *static_cast<UiValues*>(closure_context(state, first_upvalue));
}
}

UiBinding* UiValues::checked_binding(void* state, int minimum, std::uint32_t* handle) {
    const auto& api = *bridge_.api;
    if (api.gettop(state) < minimum) {
        api.lerror(state, "invalid number of arguments");
        return nullptr;
    }
    const auto index = static_cast<std::uint32_t>(api.checkinteger(state, 1));
    if (index >= bindings_.records.size()) {
        api.argerror(state, 1, "invalid item");
        return nullptr;
    }
    if (handle)
        *handle = index;
    return &bindings_.records[index];
}
ui::MenuWindow& UiValues::window() const {
    if (!bindings_.window)
        throw std::logic_error("Lua UI requires its menu window");
    return *bindings_.window;
}

int UiValues::get_widget(void* state, ui::Widget& widget) {
    const auto& api = *bridge_.api;
    switch (widget.kind) {
    case 3: { // 4341E5C6
        const auto* value = static_cast<ui::Checkbox&>(widget).value;
        api.pushboolean(state, value && *value);
        return 1;
    }
    case 4: { // 433D0A70: selected option, rather than the backing integer.
        const auto& combo = static_cast<ui::ComboBox&>(widget);
        const auto text = utf8(combo.options.at(combo.selected_item).name);
        api.pushlstring(state, text.data(), text.size());
        return 1;
    }
    case 5: { // 433CBC9B
        const auto* value = static_cast<ui::Slider&>(widget).value;
        api.pushinteger(state, value ? *value : 0);
        return 1;
    }
    case 6: { // 433876D0: an empty filtered list produces nil.
        const auto& list = static_cast<ui::Listbox&>(widget);
        if (list.filtered_indices.empty())
            api.pushnil(state);
        else
            api.pushinteger(
                state, list.value
                           ? *list.value
                           : list.options.at(static_cast<std::size_t>(list.selected_item)).value);
        return 1;
    }
    case 8: { // 433FA8AD
        const auto text = utf8(widget.name);
        api.pushlstring(state, text.data(), text.size());
        return 1;
    }
    case 9: { // 4339FC63
        const auto* value = static_cast<ui::HotkeyWidget&>(widget).value;
        if (!value)
            return 0;
        api.pushboolean(state, hotkey_active(value->binding, input_.bitmap()));
        api.pushinteger(state, static_cast<std::uint32_t>(value->binding.mode));
        const auto key = value->binding.packed >> 2;
        if (key == 27)
            api.pushnil(state);
        else
            api.pushinteger(state, key);
        return 3;
    }
    case 10: { // 433CF608 / 433FCA05: fixed 88-byte UTF-8 output buffer.
        std::array<char16_t, 65> text{};
        const auto length = static_cast<ui::Textbox&>(widget).copy_text(text);
        std::array<char, 88> output{};
        const auto converted =
            length && length < 64
                ? WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(text.data()),
                                      static_cast<int>(length + 1), output.data(),
                                      static_cast<int>(output.size()), nullptr, nullptr)
                : 0;
        if (converted < 2)
            api.pushlstring(state, "", 0);
        else
            api.pushlstring(state, output.data(), static_cast<std::size_t>(converted - 1));
        return 1;
    }
    case 12: { // 433FABB3
        const auto* value = static_cast<ui::ColorPicker&>(widget).value;
        const auto color = value ? *value : 0;
        for (unsigned shift = 0; shift != 32; shift += 8)
            api.pushinteger(state, (color >> shift) & 255u);
        return 4;
    }
    case 13: { // 4337EB20
        const auto& multi = static_cast<ui::MultiSelect&>(widget);
        const auto mask = multi.value ? *multi.value : 0;
        api.createtable(state, static_cast<int>(multi.selected_count()), 0);
        int index = 0;
        for (const auto& option : multi.options)
            if (option.mask & mask) {
                api.pushlstring(state, option.name.data(), option.name.size());
                api.rawseti(state, -2, ++index);
            }
        return 1;
    }
    default:
        return 0;
    }
}

int UiValues::get(void* state) {
    auto* record = checked_binding(state, 1);
    if (!record)
        return 0;
    if (record->widget) {
        if (const auto count = get_widget(state, *record->widget))
            return count;
        return bridge_.api->lerror(state, "invalid ui element type to ui.get");
    }
    if (record->configuration &&
        config_native_header(*record->configuration).type == ConfigType::blob) {
        const auto& value = *config_blob_value(*record->configuration);
        bridge_.api->pushlstring(state, reinterpret_cast<const char*>(value.data()), value.size());
        return 1;
    }
    return bridge_.api->argerror(state, 1, "invalid item");
}

int UiValues::set_callback(void* state) {
    std::uint32_t handle;
    auto* record = checked_binding(state, 2, &handle);
    if (!record)
        return 0;
    if (!record->widget)
        return bridge_.api->argerror(state, 1, "invalid item");
    bindings_.set_callback(bridge_, handle, 2, state);
    return 0;
}
int UiValues::set_visible(void* state) {
    std::uint32_t handle;
    auto* record = checked_binding(state, 2, &handle);
    if (!record)
        return 0;
    bool previous = false;
    if (record->widget) {
        const auto requested = bridge_.api->toboolean(state, 2) != 0;
        if (record->widget->kind == 9 && handle && bindings_.records[handle - 1].widget &&
            bindings_.records[handle - 1].widget->kind == 8)
            bindings_.records[handle - 1].widget->set_visible(requested);
        previous = record->widget->visible;
        record->widget->set_visible(requested);
    }
    bridge_.api->pushboolean(state, previous);
    return 1;
}
int UiValues::set_enabled(void* state) {
    std::uint32_t handle;
    auto* record = checked_binding(state, 2, &handle);
    if (!record)
        return 0;
    bool previous = false;
    if (record->widget) {
        const auto requested = bridge_.api->toboolean(state, 2) != 0;
        if (record->widget->kind == 9 && handle && bindings_.records[handle - 1].widget &&
            bindings_.records[handle - 1].widget->kind == 8)
            bindings_.records[handle - 1].widget->enabled = requested;
        previous = record->widget->enabled;
        record->widget->enabled = requested;
    }
    bridge_.api->pushboolean(state, previous);
    return 1;
}
int UiValues::name(void* state) {
    auto* record = checked_binding(state, 1);
    if (!record || !record->widget || record->widget->name.empty())
        return 0;
    const auto text = utf8(record->widget->name);
    bridge_.api->pushlstring(state, text.data(), text.size());
    return 1;
}
int UiValues::is_menu_open(void* state) {
    bridge_.api->pushboolean(state, (window().input.mouse_flags & 1) != 0);
    return 1;
}
int UiValues::mouse_position(void* state) {
    const auto position = window().cursor;
    bridge_.api->pushinteger(state, position.x);
    bridge_.api->pushinteger(state, position.y);
    return 2;
}
int UiValues::menu_position(void* state) {
    const auto position = window().position;
    bridge_.api->pushinteger(state, position.x);
    bridge_.api->pushinteger(state, position.y);
    return 2;
}
int UiValues::menu_size(void* state) {
    const auto size = window().geometry.size;
    bridge_.api->pushinteger(state, size.x);
    bridge_.api->pushinteger(state, size.y);
    return 2;
}
int UiValues::type(void* state) {
    auto* record = checked_binding(state, 1);
    if (!record)
        return 0;
    const auto kind = record->widget ? record->widget->kind : 0;
    const auto text = types_.name(kind);
    bridge_.api->pushlstring(state, text.data(), text.size());
    return 1;
}

int UiValues::get_entry(void* state) {
    return owner(state).get(state);
}
int UiValues::set_entry(void* state) {
    return owner(state).set(state);
}
int UiValues::callback_entry(void* state) {
    return owner(state).set_callback(state);
}
int UiValues::visible_entry(void* state) {
    return owner(state).set_visible(state);
}
int UiValues::enabled_entry(void* state) {
    return owner(state).set_enabled(state);
}
int UiValues::name_entry(void* state) {
    return owner(state).name(state);
}
int UiValues::open_entry(void* state) {
    return owner(state).is_menu_open(state);
}
int UiValues::mouse_entry(void* state) {
    return owner(state).mouse_position(state);
}
int UiValues::position_entry(void* state) {
    return owner(state).menu_position(state);
}
int UiValues::size_entry(void* state) {
    return owner(state).menu_size(state);
}
int UiValues::update_entry(void* state) {
    return owner(state).update(state);
}
int UiValues::type_entry(void* state) {
    return owner(state).type(state);
}
void UiValues::register_into(int table_index) {
    if (!bridge_.api || !bridge_.state)
        throw std::logic_error("Lua UI values require an initialized state");
    const auto& api = *bridge_.api;
    void* state = bridge_.state;
    closure_context = api.touserdata;
    const auto index =
        table_index < 0 && table_index > -10000 ? api.gettop(state) + table_index + 1 : table_index;
    for (const auto& function : std::array<std::pair<const char*, LuaApi::CFunction>, 12>{
             {{"get", get_entry},
              {"set", set_entry},
              {"set_callback", callback_entry},
              {"set_visible", visible_entry},
              {"set_enabled", enabled_entry},
              {"name", name_entry},
              {"is_menu_open", open_entry},
              {"mouse_position", mouse_entry},
              {"menu_position", position_entry},
              {"menu_size", size_entry},
              {"update", update_entry},
              {"type", type_entry}}}) {
        api.pushlightuserdata(state, this);
        api.pushcclosure(state, function.second, 1);
        api.setfield(state, index, function.first);
    }
}

} // namespace skeet::script
