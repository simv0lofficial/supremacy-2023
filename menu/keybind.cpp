// Menu Component: keybind
#include "skeet/core.hpp"
#include <stdexcept>
#include "skeet/hotkey_widget.hpp"
#include <windows.h>
#include <bit>
#include <cwchar>
#include <cwctype>


// ============================================================================
// Subcomponent: hotkey.cpp
// ============================================================================

namespace skeet {
// Original dispatch: 0x434721C4. Its external call is ntdll!RtlTestBit.
bool hotkey_active(const HotkeyBinding& binding, InputBitmap input) {
    switch (binding.mode) {
    case HotkeyMode::always: // 0x433422C6
        return true;
    case HotkeyMode::toggle: // 0x433AE8A5
        return (binding.packed & 2) != 0;
    case HotkeyMode::hold: // 0x43386E1B
    case HotkeyMode::inverse_hold: { // 0x433A1464
        const auto bit = binding.packed >> 2;
        if (bit / 32 >= input.words.size())
            throw std::out_of_range("Activation bit is outside bitmap storage");
        // RtlTestBit reads the buffer directly; SizeOfBitMap is not a bounds gate.
        const bool set = ((input.words[bit / 32] >> (bit % 32)) & 1) != 0;
        return binding.mode == HotkeyMode::hold ? set : !set;
    }
    }
    throw std::invalid_argument("Unknown activation mode");
}

} // namespace skeet

// ============================================================================
// Subcomponent: hotkey_widget.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::ui {
namespace {
using i32 = std::int32_t;
i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
// Native abbreviations at 0x43472DE0. Every byte-sized virtual key has an entry.
constexpr std::array<std::u16string_view, 256> key_names{{
    u"00",  u"M1",  u"M2",  u"CAN", u"M3",  u"M4",  u"M5",  u"07",  u"BCK", u"TAB", u"0A",  u"0B",
    u"CLR", u"ENT", u"0E",  u"0F",  u"SHF", u"CTR", u"ALT", u"PAU", u"CAP", u"15",  u"16",  u"17",
    u"18",  u"19",  u"1A",  u"ESC", u"1C",  u"1D",  u"1E",  u"1F",  u"SPC", u"PGU", u"PGD", u"END",
    u"HOM", u"LEF", u"UP",  u"RIG", u"DOW", u"SEL", u"PRI", u"EXE", u"PS",  u"INS", u"DEL", u"HLP",
    u"0",   u"1",   u"2",   u"3",   u"4",   u"5",   u"6",   u"7",   u"8",   u"9",   u"3A",  u"3B",
    u"3C",  u"3D",  u"3E",  u"3F",  u"40",  u"A",   u"B",   u"C",   u"D",   u"E",   u"F",   u"G",
    u"H",   u"I",   u"J",   u"K",   u"L",   u"M",   u"N",   u"O",   u"P",   u"Q",   u"R",   u"S",
    u"T",   u"U",   u"V",   u"W",   u"X",   u"Y",   u"Z",   u"WIN", u"WIN", u"5D",  u"5E",  u"5F",
    u"KP0", u"KP1", u"KP2", u"KP3", u"KP4", u"KP5", u"KP6", u"KP7", u"KP8", u"KP9", u"KP*", u"KP+",
    u"KPS", u"KP-", u"KP.", u"KP/", u"F1",  u"F2",  u"F3",  u"F4",  u"F5",  u"F6",  u"F7",  u"F8",
    u"F9",  u"F10", u"F11", u"F12", u"F13", u"F14", u"F15", u"F16", u"F17", u"F18", u"F19", u"F20",
    u"F21", u"F22", u"F23", u"F24", u"88",  u"89",  u"8A",  u"8B",  u"8C",  u"8D",  u"8E",  u"8F",
    u"NUM", u"SCR", u"92",  u"93",  u"94",  u"95",  u"96",  u"97",  u"98",  u"99",  u"9A",  u"9B",
    u"9C",  u"9D",  u"9E",  u"9F",  u"SHF", u"SHF", u"CTR", u"CTR", u"ALT", u"ALT", u"A6",  u"A7",
    u"A8",  u"A9",  u"AA",  u"AB",  u"AC",  u"AD",  u"AE",  u"AF",  u"B0",  u"B1",  u"B2",  u"B3",
    u"B4",  u"B5",  u"B6",  u"B7",  u"B8",  u"B9",  u";:",  u"+",   u",",   u"-",   u".",   u"/?",
    u"`~",  u"C1",  u"C2",  u"C3",  u"C4",  u"C5",  u"C6",  u"C7",  u"C8",  u"C9",  u"CA",  u"CB",
    u"CC",  u"CD",  u"CE",  u"CF",  u"D0",  u"D1",  u"D2",  u"D3",  u"D4",  u"D5",  u"D6",  u"D7",
    u"D8",  u"D9",  u"DA",  u"[{",  u"\\|", u"]}",  u"'\"", u"DF",  u"E0",  u"E1",  u"E2",  u"E3",
    u"E4",  u"E5",  u"E6",  u"E7",  u"E8",  u"E9",  u"EA",  u"EB",  u"EC",  u"ED",  u"EE",  u"EF",
    u"F0",  u"F1",  u"F2",  u"F3",  u"F4",  u"F5",  u"F6",  u"F7",  u"F8",  u"F9",  u"PLA", u"ZOO",
    u"FC",  u"FD",  u"FE",  u"FF",
}};
}
HotkeyWidget::HotkeyWidget() {
    kind = 9;
    inline_element = true;
}
HotkeyWidget::HotkeyWidget(Widget* owner, RegisteredHotkey* setting, bool allow_mode,
                           WidgetIdentities& identities) // 0x4334DD58
    : Widget(owner, u"", 9, setting != nullptr, identities), value(setting) {
    inline_element = true;
    geometry.default_size = {17, 11};
    geometry.bounds.width = 17;
    geometry.bounds.height = 11;
    if (allow_mode) {
        popup = std::make_unique<PopupMenu>(
            this, std::vector<std::u16string>{u"Always on", u"On hotkey", u"Toggle", u"Off hotkey"},
            identities);
        popup->selected = mode_selected;
        popup->geometry.bounds.x = add(popup->geometry.bounds.x, popup->geometry.horizontal_offset);
        popup->geometry.bounds.y = 12;
        if (value)
            popup->set_selected(std::uint32_t(value->binding.mode));
    }
    if (value)
        set_key(value->binding.packed >> 2);
}
void HotkeyWidget::mode_selected(Widget& owner, PopupMenu& menu,
                                 std::uint32_t index) { // 0x4331FFE8
    auto& hotkey = static_cast<HotkeyWidget&>(owner);
    if (hotkey.value) {
        hotkey.value->binding.mode = HotkeyMode(index);
        menu.set_selected(index);
    }
}
void HotkeyWidget::set_mode(HotkeyMode mode) {
    if (value) {
        value->binding.mode = mode;
        if (popup)
            popup->set_selected(std::uint32_t(mode));
    }
}
void HotkeyWidget::set_key(std::uint32_t key, std::uint32_t key_data) { // 0x4341859F
    static_assert(sizeof(wchar_t) == 2);
    if (value && (value->binding.packed >> 2) != key) {
        value->binding.packed = (value->binding.packed & 3U) | (key << 2);
        value_changed();
    }
    key_text.fill(0);
    if (!key || key == VK_ESCAPE) {
        key_text[0] = L'-';
        return;
    }
    if (key < key_names.size()) {
        const auto abbreviation = key_names[key];
        for (std::size_t i = 0; i < abbreviation.size() && i < 3; ++i)
            key_text[i] = wchar_t(abbreviation[i]);
        return;
    }
    if (!key_data) {
        key_data = (MapVirtualKeyExW(key, 0, nullptr) & 0xffU) << 16;
        if ((key >= VK_PRIOR && key <= VK_DOWN) || (key > VK_SNAPSHOT && key <= VK_DELETE) ||
            key == VK_DIVIDE || key == VK_NUMLOCK)
            key_data |= 0x01000000U;
    }
    key_data = (key_data & 0xfbffffffU) | 0x02000000U;
    std::array<wchar_t, 256> full_name{};
    if (!(key_data & 0x00ff0000U) || !GetKeyNameTextW(LONG(key_data), full_name.data(), 255))
        swprintf_s(full_name.data(), full_name.size(), L"%u", key);
    for (std::size_t i = 0; i < 3 && full_name[i]; ++i)
        if (iswalpha(full_name[i]))
            key_text[i] = wchar_t(towupper(full_name[i]));
}
bool HotkeyWidget::hit_test(Point cursor) const { // 0x433AD751
    if (popup && popup->hit_test(cursor))
        return true;
    const auto& bounds = geometry.bounds;
    return sub(bounds.x, bounds.width) <= cursor.x && cursor.x <= add(bounds.x, 25) &&
           bounds.y <= cursor.y && cursor.y <= add(bounds.y, bounds.height);
}
bool HotkeyWidget::mouse(const InputMessage& message, Point cursor) { // 0x4332604C
    if (popup) {
        if (popup->mouse(message, cursor))
            return true;
        if (popup->open)
            return false;
    }
    if (message.message == 0x200)
        hovered = hit_test(cursor);
    else if (message.message == 0x201) {
        if (hit_test(cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x202 && held) {
        const auto inside = hit_test(cursor);
        held = false;
        if (inside) {
            capturing = true;
            return true;
        }
    } else if (message.message == 0x204 && popup) {
        if (contains(geometry.bounds, cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x205 && popup && held) {
        const auto inside = contains(geometry.bounds, cursor);
        held = false;
        if (inside) {
            popup->open = true;
            return true;
        }
    }
    return false;
}
bool HotkeyWidget::keyboard(const InputMessage& message) { // 0x43328F7C
    if (!capturing)
        return false;
    std::uint32_t key, key_data = 0;
    switch (message.message) {
    case WM_KEYDOWN:
        key = message.parameter;
        key_data = message.key_data;
        break;
    case WM_LBUTTONDOWN:
        key = VK_LBUTTON;
        break;
    case WM_RBUTTONDOWN:
        key = VK_RBUTTON;
        break;
    case WM_MBUTTONDOWN:
        key = VK_MBUTTON;
        break;
    case WM_XBUTTONDOWN:
        key = (message.parameter >> 16) + 4;
        break;
    default:
        return false;
    }
    set_key(key, key_data);
    capturing = false;
    return true;
}
void HotkeyWidget::config_loaded() { // 0x433ADDF2
    if (!value)
        return;
    if (popup)
        mode_selected(*this, *popup, std::uint32_t(value->binding.mode));
    set_key(value->binding.packed >> 2);
}
void HotkeyWidget::value_changed() { // 0x433B1ED1
    notify_callbacks();
    config_loaded();
}
void HotkeyWidget::reset_input() { // 0x4336B82D
    hovered = false;
    if (popup)
        popup->reset_input();
}
std::int32_t HotkeyWidget::input_priority() const { // 0x43328387
    return popup && popup->open ? 2 : 0;
}
void HotkeyWidget::dpi_changed(float scale, const SliderMetrics& metrics) { // 0x433E51C2
    resize_element(geometry, scale);
    if (popup)
        popup->dpi_changed(scale, metrics);
}
void HotkeyWidget::draw(RenderQueue& queue, Point origin,
                        std::uint32_t accent) const { // 0x43330CC3
    const Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    if (key_text[0]) {
        std::u16string text(u"[");
        for (const auto ch : key_text) {
            if (!ch)
                break;
            text.push_back(char16_t(ch));
        }
        text.push_back(u']');
        queue.text(add(position.x, geometry.bounds.width), add(position.y, 1),
                   capturing ? 0xff0000ffU
                   : hovered ? 0x82ffffffU
                             : 0x64ffffffU,
                   0x84, 0, text);
    }
    if (popup)
        popup->draw(queue, position, accent);
}
} // namespace skeet::ui
