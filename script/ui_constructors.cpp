#include "skeet/script/ui_module.hpp"
#include "skeet/script/ui_type_storage.hpp"
#include "skeet/button.hpp"
#include "skeet/color_picker.hpp"
#include "skeet/combobox.hpp"
#include "skeet/config_storage/storage.hpp"
#include "skeet/hotkey_widget.hpp"
#include "skeet/listbox.hpp"
#include "skeet/multiselect.hpp"
#include "skeet/textbox.hpp"
#include <algorithm>
#include <cstring>
#include <cwchar>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
namespace {
std::u16string decode_range(std::string_view bytes) { // 0x4332791E
    if (bytes.empty())
        return {};
    const auto size = static_cast<int>(bytes.size());
    const auto count = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), size, nullptr, 0);
    if (count == 0)
        return {};
    std::u16string text(static_cast<std::size_t>(count), u'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), size, reinterpret_cast<wchar_t*>(text.data()),
                        count);
    return text;
}
std::uint32_t string_hash(const char* text) noexcept { // 0x43359C70
    std::uint32_t hash = 0x811c9dc5;
    for (const auto* byte = reinterpret_cast<const unsigned char*>(text); *byte; ++byte)
        hash = (hash ^ *byte) * 0x01000193u;
    return hash ^ 0x92ca5df1u;
}
template <class Function>
void visit_options(const LuaApi& api, void* state, bool strict, Function&& visit) {
    const auto arguments = api.gettop(state);
    if (api.type(state, 4) == 5) {
        api.pushvalue(state, 4);
        const auto count = static_cast<std::uint32_t>(api.objlen(state, -1));
        for (std::uint32_t index = 1; index <= count; ++index) {
            api.rawgeti(state, -1, static_cast<int>(index));
            std::size_t size = 0;
            const char* text =
                strict ? api.checklstring(state, -1, &size) : api.tolstring(state, -1, &size);
            if (text)
                visit(text, size, index - 1, true);
            api.settop(state, -2);
        }
        api.settop(state, -2);
    } else {
        for (int index = 4; index <= arguments; ++index) {
            std::size_t size = 0;
            const char* text =
                strict ? api.checklstring(state, index, &size) : api.tolstring(state, index, &size);
            if (text)
                visit(text, size, static_cast<std::uint32_t>(index - 4), false);
        }
    }
}
bool equal_widget_name(std::u16string_view name, const char16_t* requested) {
    // 433B846C supplies 128 units to 433FAB07. Its 43418122 decoder leaves
    // the zeroed destination empty when the complete name does not fit.
    std::array<wchar_t, 128> text{};
    if (name.size() < text.size() && !name.empty())
        std::memcpy(text.data(), name.data(), name.size() * sizeof(char16_t));
    return _wcsicmp(text.data(), reinterpret_cast<const wchar_t*>(requested)) == 0;
}
}

int UiModule::validate_options(void* state, bool empty_table_is_argument_error) {
    const auto& api = *bridge_.api;
    if (api.type(state, 4) == 5) {
        if (api.objlen(state, 4) == 0) {
            return empty_table_is_argument_error ? api.argerror(state, 4, "invalid options")
                                                 : api.lerror(state, "invalid number of arguments");
        }
    } else {
        const auto count = api.gettop(state);
        for (int index = 4; index <= count; ++index)
            api.checklstring(state, index, nullptr);
    }
    return 0;
}

int UiModule::new_combobox(void* state) { // 0x433E160D
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 4)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    if (const auto error = validate_options(state, true))
        return error;
    auto config = std::make_unique<IntegerConfig>();
    initialize_integer_config(configuration_.descriptors, *config, target.hash, 0);
    auto widget =
        std::make_unique<ui::ComboBox>(target.category, std::u16string_view(target.label.data()),
                                       &config->value, false, identities_);
    auto* combo = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *combo);
    visit_options(api, state, true, [&](const char* text, std::size_t size, std::uint32_t, bool) {
        auto bytes = std::string_view(text, size);
        bytes = bytes.substr(0, bytes.find('\n')); // 0x4334DC68
        if (!bytes.empty()) {
            const auto value = static_cast<std::uint32_t>(combo->options.size());
            combo->options.push_back({decode_range(bytes), value});
            if (*combo->value == value)
                combo->selected_item = value;
            combo->update_scroll();
        }
    });
    combo->set_value(0);
    return finish(state, target, *combo);
}

int UiModule::new_multiselect(void* state) { // 0x433D0B9A
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 4)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    if (const auto error = validate_options(state, false))
        return error;
    auto config = std::make_unique<IntegerConfig>();
    initialize_integer_config(configuration_.descriptors, *config, target.hash, 0);
    auto widget =
        std::make_unique<ui::MultiSelect>(target.category, std::u16string_view(target.label.data()),
                                          &config->value, identities_, false);
    auto* multiple = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *multiple);
    visit_options(api, state, true, [&](const char* text, std::size_t size, std::uint32_t, bool) {
        auto bytes = std::string_view(text, size);
        bytes = bytes.substr(0, bytes.find('\n'));
        if (!bytes.empty() && multiple->options.size() < 32) {
            const auto mask = std::uint32_t(1) << multiple->options.size();
            multiple->options.push_back({std::string(bytes), mask});
            multiple->refresh_display();
            multiple->update_scroll();
        }
    });
    multiple->set_value(0);
    return finish(state, target, *multiple);
}

int UiModule::new_hotkey(void* state) { // 0x433E9810
    const auto& api = *bridge_.api;
    const auto count = api.gettop(state);
    if (count < 3)
        return api.lerror(state, "invalid number of arguments");
    const auto key = count > 4 ? static_cast<std::uint32_t>(api.checkinteger(state, 5)) : 27u;
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    if (count < 4 || !api.toboolean(state, 4)) {
        auto label = std::make_unique<ui::Label>(
            target.category, std::u16string_view(target.label.data()), identities_);
        auto* text = label.get();
        bindings_.adopt(std::move(label));
        attach(target, *text);
    }
    auto config = std::make_unique<HotkeyConfig>();
    initialize_hotkey_config(configuration_, *config, target.hash, key, HotkeyMode::hold);
    auto widget =
        std::make_unique<ui::HotkeyWidget>(target.category, &config->value, true, identities_);
    auto* hotkey = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *hotkey);
    return finish(state, target, *hotkey);
}

int UiModule::new_button(void* state) { // 0x43365BC4
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 4)
        return api.lerror(state, "invalid number of arguments");
    api.checktype(state, 4, 6);
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    const auto next_handle = static_cast<std::uint32_t>(bindings_.records.size());
    const auto callback = bindings_.create_callback(bridge_, next_handle, 4, state);
    auto widget =
        std::make_unique<ui::Button>(target.category, std::u16string_view(target.label.data()),
                                     Point{158, 28}, callback.callback, identities_);
    auto* button = widget.get();
    const auto handle = bindings_.adopt(std::move(widget));
    bindings_.records[handle].callback_references.push_back(callback.reference);
    attach(target, *button);
    return finish(state, target, *button);
}

int UiModule::new_color_picker(void* state) { // 0x43342C55
    const auto& api = *bridge_.api;
    const auto count = api.gettop(state);
    if (count < 3)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    std::uint32_t red = 255, green = 0, blue = 0, alpha = 255;
    if (count >= 6) {
        red = static_cast<std::uint32_t>(api.checkinteger(state, 4));
        green = static_cast<std::uint32_t>(api.checkinteger(state, 5));
        blue = static_cast<std::uint32_t>(api.checkinteger(state, 6));
        if (count >= 7)
            alpha = static_cast<std::uint32_t>(api.checkinteger(state, 7));
    }
    auto config = std::make_unique<ColorConfig>();
    initialize_color_config(configuration_.descriptors, *config, target.hash,
                            (red & 255) | ((green & 255) << 8) | ((blue & 255) << 16) |
                                (alpha << 24));
    auto widget = std::make_unique<ui::ColorPicker>(target.category, &config->value, identities_);
    auto* picker = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *picker);
    return finish(state, target, *picker);
}

int UiModule::new_textbox(void* state) { // 0x433B0FE0
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 2)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    auto config = std::make_unique<BooleanConfig>();
    initialize_boolean_config(configuration_.descriptors, *config, target.hash);
    auto widget = std::make_unique<ui::Textbox>(target.category, identities_);
    auto* textbox = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *textbox);
    return finish(state, target, *textbox);
}

int UiModule::new_listbox(void* state) { // 0x434032B6
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 4)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    auto config = std::make_unique<IntegerConfig>();
    initialize_integer_config(configuration_.descriptors, *config, target.hash, 0);
    auto widget =
        std::make_unique<ui::Listbox>(target.category, std::u16string_view(target.label.data()),
                                      Point{158, 300}, &config->value, true, identities_);
    auto* list = widget.get();
    bindings_.adopt(std::move(widget), std::move(config));
    attach(target, *list);
    std::vector<ui::ListboxOption> options;
    visit_options(api, state, false,
                  [&](const char* text, std::size_t, std::uint32_t original, bool table) {
                      std::array<wchar_t, 255> wide{};
                      if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wide.data(),
                                              static_cast<int>(wide.size())) <= 1)
                          return;
                      wide.back() = 0;
                      if (auto* newline = std::wcschr(wide.data(), L'\n'))
                          *newline = 0;
                      if (!wide[0])
                          return;
                      const auto value = table ? static_cast<std::int32_t>(options.size())
                                               : static_cast<std::int32_t>(original);
                      options.push_back(
                          {value, std::u16string(reinterpret_cast<const char16_t*>(wide.data()))});
                  });
    list->set_options(&options);
    list->set_selection(0);
    list->update_scroll();
    return finish(state, target, *list);
}

int UiModule::new_string(void* state) { // 0x433CBCD0
    const auto& api = *bridge_.api;
    const auto count = api.gettop(state);
    if (count < 1)
        return api.lerror(state, "invalid number of arguments");
    const auto* name = api.checklstring(state, 1, nullptr);
    if (!name || !*name)
        return api.argerror(state, 1, "empty string");
    const auto hash = string_hash(name);
    // Native appends its28-byte UI record before the real32-byte descriptor
    // allocation. Publish ownership before checking the optional Lua value.
    const auto handle = bindings_.adopt({});
    auto config = NativeConfigBlobOwner::create(
        configuration_, types_.memory(), types_.image().native_cell<std::byte>(0x434759ACu), hash);
    auto* value = config ? &config->configuration() : nullptr;
    auto& record = bindings_.records[handle];
    record.configuration = value;
    record.owned_configuration = std::move(config);
    if (count > 1) {
        std::size_t size = 0;
        const auto* initial = api.checklstring(state, 2, &size);
        if (initial && size) {
            const auto* first = reinterpret_cast<const std::byte*>(initial);
            value->value.assign(first, first + size);
        }
    }
    api.pushinteger(state, static_cast<std::ptrdiff_t>(bindings_.records.size() - 1));
    return 1;
}

int UiModule::new_label(void* state) { // 0x4341CF96
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 3)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    auto widget = std::make_unique<ui::Label>(
        target.category, std::u16string_view(target.label.data()), identities_);
    auto* label = widget.get();
    bindings_.adopt(std::move(widget));
    attach(target, *label);
    return finish(state, target, *label);
}

int UiModule::reference(void* state) { // 0x433B846C
    const auto& api = *bridge_.api;
    if (api.gettop(state) < 3)
        return api.lerror(state, "invalid number of arguments");
    Target target;
    if (const auto error = parse_target(state, target))
        return error;
    auto& elements = target.category->elements;
    auto found = std::find_if(elements.begin(), elements.end(), [&](const ui::Widget* widget) {
        return equal_widget_name(widget->name, target.label.data());
    });
    if (found != elements.end() && (*found)->kind == 8)
        ++found;
    if (found == elements.end())
        return api.lerror(state, "not found");
    auto supported = [](const ui::Widget& widget) {
        return (0x377cu & (1u << (widget.kind & 31))) != 0;
    };
    if (!supported(**found))
        return api.lerror(state, "control type not supported");
    int results = 0;
    do {
        api.pushinteger(state, bindings_.reference(**found));
        ++results;
        ++found;
    } while (found != elements.end() && supported(**found) && (*found)->name.empty());
    return results;
}

} // namespace skeet::script
