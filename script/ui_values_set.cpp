#include "skeet/script/ui_values.hpp"
#include "skeet/combobox.hpp"
#include "skeet/color_picker.hpp"
#include "skeet/hotkey_widget.hpp"
#include "skeet/listbox.hpp"
#include "skeet/multiselect.hpp"
#include "skeet/textbox.hpp"
#include "ui_values_detail.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <emmintrin.h>

namespace skeet::script {
namespace {
using namespace ui_values_detail;
}

int UiValues::set_widget(void* state, ui::Widget& widget, int arguments) {
    const auto& api = *bridge_.api;
    // Native 43474890 stores the minimum count beside each dispatcher.
    constexpr std::array<int, 15> minimum_arguments{0, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 0, 5, 2, 0};
    if (widget.kind >= minimum_arguments.size() || arguments < minimum_arguments[widget.kind])
        return 0;
    switch (widget.kind) {
    case 2: // 433A13BA -> 4334E09F
        widget.notify_callbacks();
        return 0;
    case 3: // 433F111E
        api.checktype(state, 2, 1);
        static_cast<ui::Checkbox&>(widget).set_value(api.toboolean(state, 2) != 0);
        return 0;
    case 4: { // 4339F8A8: isstring is tested before isnumber.
        auto& combo = static_cast<ui::ComboBox&>(widget);
        if (api.isstring(state, 2)) {
            const auto text = utf16(api.checklstring(state, 2, nullptr));
            for (const auto& option : combo.options)
                if (option.name == text) {
                    combo.set_value(option.value);
                    return 0;
                }
        } else if (api.isnumber(state, 2)) {
            const auto value = static_cast<std::uint32_t>(api.checkinteger(state, 2));
            for (const auto& option : combo.options)
                if (option.value == value) {
                    combo.set_value(option.value);
                    return 0;
                }
        }
        return api.argerror(state, 2, "value not found");
    }
    case 5: { // 43365D18
        auto& slider = static_cast<ui::Slider&>(widget);
        const auto value = static_cast<std::int32_t>(api.checkinteger(state, 2));
        if (value < slider.minimum || value > slider.maximum)
            return api.argerror(state, 2, "out of bounds");
        slider.set_value(value);
        return 0;
    }
    case 6: { // 433B49A8
        auto& list = static_cast<ui::Listbox&>(widget);
        const auto value = static_cast<std::int32_t>(api.checkinteger(state, 2));
        for (std::size_t index = 0; index < list.options.size(); ++index) {
            if (list.options[index].value != value)
                continue;
            if ((list.value ? *list.value : 0) != value) {
                if (list.value)
                    *list.value = value;
                list.value_changed();
            }
            list.set_selection(static_cast<std::uint32_t>(index));
            break;
        }
        return 0;
    }
    case 8: // 433CC845
        widget.name = utf16(api.checklstring(state, 2, nullptr));
        return 0;
    case 9: { // 433F1D59
        auto& hotkey = static_cast<ui::HotkeyWidget&>(widget);
        auto apply_mode = [&](HotkeyMode mode) { // 433E3D86
            if (hotkey.value)
                hotkey.value->binding.mode = mode;
            if (hotkey.popup)
                hotkey.popup->set_selected(static_cast<std::uint32_t>(mode));
        };
        if (arguments >= 2 && api.isstring(state, 2)) {
            std::size_t length = 0;
            const auto* text = api.checklstring(state, 2, &length);
            switch (mode_hash(text, length)) {
            case 0x3610b364:
                apply_mode(HotkeyMode::always);
                break;
            case 0x7a22ddab:
                apply_mode(HotkeyMode::hold);
                break;
            case 0xd2e33ff4:
                apply_mode(HotkeyMode::toggle);
                break;
            case 0x5dded141:
                apply_mode(HotkeyMode::inverse_hold);
                break;
            default:
                break;
            }
        }
        if (arguments >= 3 && api.isnumber(state, 3)) {
            const auto key = static_cast<std::uint32_t>(api.checkinteger(state, 3));
            if (key < 256 && hotkey.value) {
                // Native always invokes value_changed, even for the same key.
                hotkey.value->binding.packed = (hotkey.value->binding.packed & 3u) | (key << 2);
                hotkey.value_changed();
            }
        }
        return 0;
    }
    case 10: { // 433F51D0 / 4333C5F3
        auto& textbox = static_cast<ui::Textbox&>(widget);
        std::size_t bytes = 0;
        const char* value = api.checklstring(state, 2, &bytes);
        if (bytes >= 64)
            return 0;
        if (!bytes) {
            textbox.text_length.exchange(0);
            return 0;
        }
        std::array<wchar_t, 128> text{};
        const auto converted = MultiByteToWideChar(CP_UTF8, 0, value, -1, text.data(), 127);
        const auto length = converted > 1 ? converted - 1 : 0;
        single_line(text);
        if (!length || length >= 64)
            return 0;
        while (textbox.text_lock.exchange(1))
            _mm_pause();
        textbox.text_length.exchange(static_cast<std::uint32_t>(length));
        std::copy_n(reinterpret_cast<const char16_t*>(text.data()), length, textbox.text.begin());
        textbox.text[static_cast<std::size_t>(length)] = 0;
        textbox.text_lock.exchange(0);
        return 0;
    }
    case 12: { // 4331E000: components are narrowed, never clamped.
        std::uint32_t color = 0;
        for (int index = 2; index <= 5; ++index)
            color |= (static_cast<std::uint32_t>(api.checkinteger(state, index)) & 255u)
                     << ((index - 2) * 8);
        static_cast<ui::ColorPicker&>(widget).set_value(color);
        return 0;
    }
    case 13: { // 433EB931
        auto& multi = static_cast<ui::MultiSelect&>(widget);
        std::uint32_t mask = 0;
        auto add = [&](const char* text) {
            for (const auto& option : multi.options)
                if (!std::strcmp(option.name.c_str(), text)) {
                    mask |= option.mask;
                    break;
                }
        };
        if (arguments == 2 && api.type(state, 2) == 5) {
            api.pushvalue(state, 2);
            const auto count = api.objlen(state, -1);
            for (std::size_t index = 1; index <= count; ++index) {
                api.rawgeti(state, -1, static_cast<int>(index));
                if (const char* text = api.checklstring(state, -1, nullptr))
                    add(text);
                api.settop(state, -2);
            }
            api.settop(state, -2);
        } else {
            for (int index = 2; index <= arguments; ++index)
                if (const char* text = api.checklstring(state, index, nullptr))
                    add(text);
        }
        if (multi.require_at_least_one && !mask)
            return api.argerror(state, 2, "this multiselect requires at least one value");
        multi.set_value(mask);
        return 0;
    }
    default:
        return 0;
    }
}

int UiValues::set(void* state) {
    const auto& api = *bridge_.api;
    const auto arguments = api.gettop(state);
    if (arguments < 1)
        return 0;
    auto* record = checked_binding(state, 1);
    if (!record)
        return 0;
    if (record->widget)
        return set_widget(state, *record->widget, arguments);
    if (!record->configuration ||
        config_native_header(*record->configuration).type != ConfigType::blob)
        return api.argerror(state, 1, "invalid item");
    if (arguments > 1) {
        std::size_t length = 0;
        const char* text = api.checklstring(state, 2, &length);
        if (text && length) {
            auto& value = *config_blob_value(*record->configuration);
            value.assign(reinterpret_cast<const std::byte*>(text),
                         reinterpret_cast<const std::byte*>(text) + length);
        }
    }
    return 0;
}

int UiValues::update(void* state) {
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 2)
        return 0;
    const auto handle = static_cast<std::uint32_t>(api.checkinteger(state, 1));
    if (handle >= bindings_.records.size())
        return 0;
    auto& record = bindings_.records[handle];
    if (record.borrowed || !record.widget || record.widget->kind != 6 || api.type(state, 2) != 5)
        return 0;
    const auto count = api.objlen(state, 2);
    std::vector<ui::ListboxOption> replacement;
    replacement.reserve(count);
    for (std::size_t index = 1; index <= count; ++index) {
        api.rawgeti(state, 2, static_cast<int>(index));
        if (const char* value = api.tolstring(state, -1, nullptr)) {
            std::array<wchar_t, 256> text{};
            const auto converted = MultiByteToWideChar(CP_UTF8, 0, value, -1, text.data(), 255);
            single_line(text);
            if (converted > 1 && text[0])
                replacement.push_back(
                    {static_cast<std::int32_t>(replacement.size()),
                     std::u16string(reinterpret_cast<const char16_t*>(text.data()))});
        }
        api.settop(state, -2);
    }
    static_cast<ui::Listbox&>(*record.widget).set_options(&replacement);
    return 0;
}

} // namespace skeet::script
