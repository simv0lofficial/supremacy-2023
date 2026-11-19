// Menu Component: colorpicker
#include "skeet/color_picker.hpp"
#include <windows.h>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <immintrin.h>
#include <stdexcept>
#include "skeet/client_menu.hpp"
#include "config_binding.hpp"
#include <algorithm>


// ============================================================================
// Subcomponent: color_picker.cpp
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
i32 truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
std::uint32_t swap_bytes(std::uint32_t value) noexcept {
    return __builtin_bswap32(value);
}
std::uint32_t color_byte(float value) noexcept {
    const auto scaled = value * 255.0f;
    const auto wide = double(scaled);
    if (!std::isfinite(scaled) || wide < -9223372036854775808.0 || wide >= 9223372036854775808.0)
        return 0;
    return std::uint32_t(std::int64_t(wide)) & 255;
}
float unit(float value) noexcept {
    if (value > 1.0f)
        return 1.0f;
    return value < 0.0f ? 0.0f : value;
}
void clipboard_write(void*, std::uint32_t color) { // 0x433F0C77
    if (!OpenClipboard(nullptr))
        return;
    if (EmptyClipboard()) {
        const auto memory = GlobalAlloc(0, 9);
        if (memory) {
            const auto text = format_color_clipboard(color);
            std::memcpy(memory, text.data(), text.size());
            if (!SetClipboardData(CF_TEXT, memory))
                GlobalFree(memory);
        }
    }
    CloseClipboard();
}
bool clipboard_read(void*, std::uint32_t& color) { // 0x43364AE2
    if (!IsClipboardFormatAvailable(CF_TEXT) || !OpenClipboard(nullptr))
        return false;
    bool result = false;
    if (const auto memory = GetClipboardData(CF_TEXT)) {
        if (const auto text = static_cast<char*>(GlobalLock(memory))) {
            result = parse_color_clipboard(text, color);
            GlobalUnlock(text);
        }
    }
    CloseClipboard();
    return result;
}
}
std::array<char, 9>
format_color_clipboard(std::uint32_t color) noexcept { // 0x433F0C77 text formatting
    std::array<char, 9> text{};
    std::snprintf(text.data(), text.size(), "%08X", color);
    return text;
}
bool parse_color_clipboard(const char* text,
                           std::uint32_t& color) noexcept { // 0x43364AE2 text parsing
    if (std::strlen(text) != 8)
        return false;
    color = std::uint32_t(std::strtoul(text, nullptr, 16));
    return true;
}
ColorClipboard& system_color_clipboard() noexcept {
    static ColorClipboard clipboard{nullptr, clipboard_write, clipboard_read};
    return clipboard;
}
HsvColor rgb_to_hsv(std::uint32_t color) noexcept { // 0x433E7FEC
    const auto r = float(color & 255) * 0.003921568859368563f;
    const auto g = float((color >> 8) & 255) * 0.003921568859368563f;
    const auto b = float((color >> 16) & 255) * 0.003921568859368563f;
    auto lowest = g <= r ? g : r;
    if (b <= lowest)
        lowest = b;
    auto highest = r <= g ? g : r;
    if (highest <= b)
        highest = b;
    const auto delta = highest - lowest;
    HsvColor result{0, 0, highest};
    if (delta < 0.00001f)
        return result;
    if (highest <= 0.0f) {
        result.hue = std::bit_cast<float>(0xffc00000U);
        return result;
    }
    result.saturation = delta / highest;
    if (r < highest)
        result.hue = g < highest ? (r - g) / delta + 4.0f : (b - r) / delta + 2.0f;
    else
        result.hue = (g - b) / delta;
    result.hue *= 60.0f;
    if (result.hue < 0.0f)
        result.hue += 360.0f;
    return result;
}
std::uint32_t hsv_to_rgb(HsvColor color) noexcept { // 0x43386FB2
    float r = color.value, g = color.value, b = color.value;
    if (!(color.saturation <= 0.0f)) {
        auto hue = color.hue >= 360.0f ? 0.0f : color.hue;
        hue /= 60.0f;
        const auto sector = truncate(hue);
        const auto fraction = hue - float(sector);
        const auto low = (1.0f - color.saturation) * color.value;
        const auto falling = (1.0f - fraction * color.saturation) * color.value;
        const auto rising = (1.0f - (1.0f - fraction) * color.saturation) * color.value;
        switch (sector) {
        case 0:
            g = rising;
            b = low;
            break;
        case 1:
            r = falling;
            b = low;
            break;
        case 2:
            r = low;
            b = rising;
            break;
        case 3:
            r = low;
            g = falling;
            break;
        case 4:
            r = rising;
            g = low;
            break;
        default:
            g = low;
            b = falling;
            break;
        }
    }
    return 0xff000000U | color_byte(r) | (color_byte(g) << 8) | (color_byte(b) << 16);
}
ColorPicker::ColorPicker() {
    kind = 12;
    inline_element = true;
    popup.parent = this;
    popup.selected = menu_selected;
}
ColorPicker::ColorPicker(Widget* owner, std::uint32_t* setting, WidgetIdentities& identities)
    : Widget(owner, u"", 12, setting != nullptr, identities), value(setting),
      popup(this, {u"Copy", u"Paste", u"Reset"}, identities),
      default_color_(setting ? *setting : 0) { // 0x43406A63
    inline_element = true;
    geometry.default_size = {17, 11};
    geometry.bounds.width = 17;
    geometry.bounds.height = 11;
    enabled = true;
    popup.selected = menu_selected;
    if (setting)
        config_loaded();
    popup.geometry.bounds.x = add(popup.geometry.bounds.x, popup.geometry.horizontal_offset);
    popup.geometry.bounds.y = 12;
}
void ColorPicker::set_value(std::uint32_t requested) { // 0x433F49B3
    if ((value ? *value : 0) != requested) {
        if (value)
            *value = requested;
        value_changed();
    }
}
void ColorPicker::config_loaded() {
    hsv = rgb_to_hsv(value ? *value : 0);
} // 0x433D72C9
void ColorPicker::reset_input() { // 0x43364A7C
    hovered = false;
    held = open = false;
    status = PickerStatus::idle;
    popup.reset_input();
}
i32 ColorPicker::input_priority() const {
    return open || popup.open ? 2 : 0;
} // 0x433CB533
bool ColorPicker::hit_test(Point cursor) const { // 0x433E08B0
    if (contains(geometry.bounds, cursor))
        return true;
    if (!open)
        return popup.hit_test(cursor);
    auto bounds = picker_bounds;
    bounds.x = add(bounds.x, geometry.bounds.x);
    bounds.y = add(bounds.y, geometry.bounds.y);
    return contains(bounds, cursor);
}
PickerStatus ColorPicker::region_at(Point cursor) const { // 0x43347C72
    if (contains(geometry.bounds, cursor))
        return PickerStatus::clicked;
    cursor.x = sub(cursor.x, geometry.bounds.x);
    cursor.y = sub(cursor.y, geometry.bounds.y);
    if (contains(tint_bounds, cursor))
        return PickerStatus::tint;
    if (contains(hue_bounds, cursor))
        return PickerStatus::hue;
    if (contains(alpha_bounds, cursor))
        return PickerStatus::alpha;
    return PickerStatus::idle;
}
void ColorPicker::update_at(Point cursor, PickerStatus region) { // 0x433647A9
    if (!value)
        throw std::logic_error("Color picker input requires a bound color");
    auto alpha = *value >> 24;
    if (region == PickerStatus::tint) {
        hsv.saturation = unit(float(sub(sub(cursor.x, tint_bounds.x), geometry.bounds.x)) /
                              float(tint_bounds.width));
        hsv.value = 1.0f - unit(float(sub(sub(cursor.y, tint_bounds.y), geometry.bounds.y)) /
                                float(tint_bounds.height));
    } else if (region == PickerStatus::hue) {
        const auto fraction = unit(float(sub(sub(cursor.y, hue_bounds.y), geometry.bounds.y)) /
                                   float(hue_bounds.height));
        hsv.hue = 360.0f - fraction * 360.0f;
    } else if (region == PickerStatus::alpha) {
        auto requested = truncate((float(sub(sub(cursor.x, alpha_bounds.x), geometry.bounds.x)) /
                                   float(alpha_bounds.width)) *
                                      255.0f +
                                  0.5f);
        if (requested < 0)
            requested = 0;
        else if (requested >= 251)
            requested = 255;
        alpha = std::uint32_t(requested);
    }
    set_value((hsv_to_rgb(hsv) & 0x00ffffffU) | (alpha << 24));
}
bool ColorPicker::mouse(const InputMessage& message, Point cursor) { // 0x433C88CF
    if (popup.mouse(message, cursor))
        return true;
    if (popup.open)
        return false;
    if (message.message == 0x200) {
        if ((hit_test(cursor) || held) && open) {
            if (held && status != PickerStatus::idle && status != PickerStatus::clicked)
                update_at(cursor, status);
            return true;
        }
    } else if (message.message == 0x201) {
        if (hit_test(cursor)) {
            held = true;
            status = region_at(cursor);
            if (open && (status == PickerStatus::tint || status == PickerStatus::hue ||
                         status == PickerStatus::alpha))
                update_at(cursor, status);
            return true;
        }
    } else if (message.message == 0x202) {
        if (held) {
            held = false;
            if (hit_test(cursor)) {
                if (status == PickerStatus::clicked) {
                    open = !open;
                    popup.open = false;
                    if (!open)
                        status = PickerStatus::idle;
                }
                return true;
            }
        }
    } else if (message.message == 0x204) {
        if (contains(geometry.bounds, cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x205 && held) {
        held = false;
        if (contains(geometry.bounds, cursor)) {
            popup.open = true;
            open = false;
            return true;
        }
    }
    return false;
}
void ColorPicker::dpi_changed(float scale, const SliderMetrics& metrics) { // 0x433CEE5A
    resize_element(geometry, scale);
    popup.dpi_changed(scale, metrics);
    const auto tint = scale_size({150, 150}, scale);
    tint_bounds = {5, add(geometry.bounds.height, 5), tint.x, tint.y};
    const auto hue = scale_size({15, 150}, scale);
    hue_bounds = {add(tint.x, 10), add(geometry.bounds.height, 5), hue.x, hue.y};
    const auto alpha = scale_size({150, 10}, scale);
    alpha_bounds = {5, add(add(geometry.bounds.height, tint.y), 10), alpha.x, alpha.y};
    picker_bounds = {0, geometry.bounds.height, add(add(tint.x, hue.x), 15),
                     add(add(alpha.y, tint.y), 15)};
}
void ColorPicker::menu_selected(Widget& owner, PopupMenu&, std::uint32_t index) { // 0x43417F64
    auto& picker = static_cast<ColorPicker&>(owner);
    if (!picker.value)
        return;
    if (index == 0) {
        if (!picker.clipboard || !picker.clipboard->write)
            throw std::logic_error("Color clipboard write callback is not installed");
        picker.clipboard->write(picker.clipboard->context, swap_bytes(*picker.value));
    } else if (index == 1) {
        if (!picker.clipboard || !picker.clipboard->read)
            throw std::logic_error("Color clipboard read callback is not installed");
        std::uint32_t color = 0;
        if (picker.clipboard->read(picker.clipboard->context, color)) {
            *picker.value = swap_bytes(color);
            picker.config_loaded();
            picker.value_changed();
        }
    } else if (index == 2 && *picker.value != picker.default_color_) {
        *picker.value = picker.default_color_;
        picker.config_loaded();
        picker.value_changed();
    }
}
void ColorPicker::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x4333C000
    const auto x = add(origin.x, geometry.bounds.x), y = add(origin.y, geometry.bounds.y);
    const auto selected_color = (value ? *value : 0) | 0xff000000U;
    queue.rectangle_outline(
        {add(x, 1), add(y, 1), geometry.bounds.width, sub(geometry.bounds.height, 2), 0xff0c0c0cU},
        1);
    queue.vertical_gradient({add(x, 2), add(y, 2), sub(geometry.bounds.width, 2),
                             sub(geometry.bounds.height, 4), add_color(selected_color, 0x00141414U),
                             subtract_color(selected_color, 0x00141414U)});
    if (!enabled)
        return;
    if (!open) {
        popup.draw(queue, {x, y}, accent);
        return;
    }
    const auto previous_bounds_reset = queue.is_bounds_reset();
    const auto previous_depth = queue.get_depth();
    queue.reset_bounds(false);
    queue.set_depth(0.15f);
    const auto px = add(x, picker_bounds.x), py = add(y, picker_bounds.y);
    queue.rectangle_outline({px, py, picker_bounds.width, picker_bounds.height, 0xff0c0c0cU}, 1);
    queue.rectangle_outline({add(px, 1), add(py, 1), sub(picker_bounds.width, 2),
                             sub(picker_bounds.height, 2), 0xff3c3c3cU},
                            1);
    queue.rectangle({add(px, 2), add(py, 2), sub(picker_bounds.width, 4),
                     sub(picker_bounds.height, 4), 0xff282828U});
    const auto tx = add(x, tint_bounds.x), ty = add(y, tint_bounds.y);
    queue.rectangle_outline({sub(tx, 1), sub(ty, 1), add(tint_bounds.width, 2),
                             add(tint_bounds.height, 2), 0xff0c0c0cU},
                            1);
    const auto hue_color = hsv_to_rgb({hsv.hue, 1, 1});
    queue.four_color_gradient({tx,
                               ty,
                               tint_bounds.width,
                               tint_bounds.height,
                               {0xffffffffU, hue_color, hue_color, 0xffffffffU}});
    queue.four_color_gradient(
        {tx, ty, tint_bounds.width, tint_bounds.height, {0, 0, 0xff000000U, 0xff000000U}});
    queue.outlined_rectangle(
        {add(sub(tx, 2), truncate(float(tint_bounds.width) * hsv.saturation)),
         add(sub(ty, 2), truncate((1.0f - hsv.value) * float(tint_bounds.height))), 4, 4,
         0x78ffffffU},
        0xff0c0c0cU, 1);
    const auto ax = add(x, alpha_bounds.x), ay = add(y, alpha_bounds.y);
    const auto rgba = value ? *value : 0;
    queue.outlined_rectangle(
        {sub(ax, 1), sub(ay, 1), add(alpha_bounds.width, 2), add(alpha_bounds.height, 2), rgba},
        0xff0c0c0cU, 1);
    queue.outlined_rectangle({add(sub(ax, 2), truncate((float(rgba >> 24) * 0.003921568859368563f) *
                                                           float(alpha_bounds.width) +
                                                       0.5f)),
                              ay, 4, alpha_bounds.height, 0x78ffffffU},
                             0xff0c0c0cU, 1);
    const auto hx = add(x, hue_bounds.x), hy = add(y, hue_bounds.y);
    queue.rectangle_outline(
        {sub(hx, 1), sub(hy, 1), add(hue_bounds.width, 2), add(hue_bounds.height, 2), 0xff0c0c0cU},
        1);
    constexpr std::array<std::uint32_t, 7> colors{
        0xff1e1edcU, 0xffdc1edcU, 0xffdc1e1eU, 0xffdcdc1eU, 0xff1edc1eU, 0xff1ebedcU, 0xff1e1edcU};
    const auto band_height = truncate(float(hue_bounds.height) / 6.0f + 0.5f);
    auto band_y = hy;
    for (std::size_t i = 0; i < 6; ++i) {
        queue.vertical_gradient(
            {hx, band_y, hue_bounds.width, band_height, colors[i], colors[i + 1]});
        band_y = add(band_y, band_height);
    }
    const auto marker =
        float(std::floor(double(((360.0f - hsv.hue) / 360.0f) * float(hue_bounds.height))));
    queue.outlined_rectangle(
        {hx, add(sub(hy, 2), truncate(marker)), hue_bounds.width, 4, 0x78ffffffU}, 0x96000000U, 1);
    queue.reset_bounds(previous_bounds_reset);
    queue.set_depth(previous_depth);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: colored_models_menu.cpp
// ============================================================================

namespace skeet::ui {
using namespace menu_detail;
namespace {
constexpr Requirement colored_models_settings[] = {
    {0xC51E0D3D, ConfigType::boolean}, {0xF6B67DE5, ConfigType::boolean},
    {0x1570AF7C, ConfigType::boolean}, {0x57BF65EC, ConfigType::boolean},
    {0x6670DB4D, ConfigType::boolean}, {0x96DB2E1C, ConfigType::boolean},
    {0x5E0E2558, ConfigType::boolean}, {0x8CCAF8DC, ConfigType::boolean},
    {0xE8407B64, ConfigType::boolean}, {0xDA411D58, ConfigType::boolean},
    {0x92EBAD93, ConfigType::boolean}, {0x8E0781AE, ConfigType::boolean},
    {0x67F8885C, ConfigType::boolean}, {0xE217C264, ConfigType::boolean},
    {0xFE06341D, ConfigType::color},   {0xD4699245, ConfigType::color},
    {0x9FF240E4, ConfigType::color},   {0x2CEFC91C, ConfigType::color},
    {0x04AD8938, ConfigType::color},   {0xF3D8F0D1, ConfigType::color},
    {0x9201E7ED, ConfigType::color},   {0x9436ECB4, ConfigType::color},
    {0xA98CEAE8, ConfigType::color},   {0xF50A4300, ConfigType::color},
    {0xC2DBF0A4, ConfigType::color},   {0x8B122392, ConfigType::color},
    {0x5A93F490, ConfigType::color},   {0x2926F5DE, ConfigType::color},
    {0x75A868A4, ConfigType::color},   {0x1C678B92, ConfigType::color},
    {0x83BE3FA7, ConfigType::color},   {0x6D66C717, ConfigType::color},
    {0xC2140C28, ConfigType::color},   {0x84830790, ConfigType::color},
    {0x044F7095, ConfigType::integer}, {0xE85380FA, ConfigType::integer},
    {0x3B28B0E5, ConfigType::integer}, {0x3EA36C3A, ConfigType::integer},
    {0x3FC208C1, ConfigType::integer}, {0x17C5B91A, ConfigType::integer},
    {0x1A07B48F, ConfigType::integer}, {0xB7360BA6, ConfigType::integer},
    {0x2A1FA11A, ConfigType::integer}, {0x118A8443, ConfigType::integer},
};
const std::vector<ComboBoxOption> materials = {
    {u"Default", 0}, {u"Solid", 1},  {u"Shaded", 2},   {u"Metallic", 3},
    {u"Glow", 4},    {u"Bubble", 5}, {u"Original", 6},
};
}

ColoredModelsMenu::ColoredModelsMenu(Tab& tab, ConfigRegistries& settings,
                                     WidgetIdentities& identities)
    : tab_(tab), category_(&tab, u"Colored models", {0, 12, 13, 11}, 16, true, identities) {
    require_settings(settings, colored_models_settings);
    storage_.reserve(std::size(colored_models_settings));
    bindings_.reserve(std::size(colored_models_settings));
    auto checkbox = [&](std::uint32_t hash, std::u16string_view label) -> Checkbox& {
        return add<Checkbox>(category_, hash, label, value<BooleanConfig>(settings, hash),
                             identities);
    };
    auto color = [&](std::uint32_t hash) -> ColorPicker& {
        return add<ColorPicker>(category_, hash, value<ColorConfig>(settings, hash), identities);
    };
    auto group = [&](std::size_t index, std::u16string_view label, std::uint32_t enabled_hash,
                     std::uint32_t primary_color, std::uint32_t material_hash,
                     std::uint32_t secondary_color, std::uint32_t behind_hash = 0,
                     std::uint32_t behind_color = 0,
                     std::u16string_view behind_label = u"") -> MaterialGroup& {
        auto& current = groups_[index];
        current.primary = &checkbox(enabled_hash, label);
        current.primary->callbacks.push_back({&current, enabled_changed});
        color(primary_color);
        if (behind_hash) {
            current.behind_wall = &checkbox(behind_hash, behind_label);
            current.behind_wall->callbacks.push_back({&current, enabled_changed});
            color(behind_color);
        }
        current.material =
            &add<ComboBox>(category_, material_hash, u"",
                           value<IntegerConfig>(settings, material_hash), false, identities);
        current.material->set_options(materials);
        current.material->callbacks.push_back({&current, material_changed});
        current.secondary_color = &color(secondary_color);
        return current;
    };
    auto& player = group(0, u"Player", 0xC51E0D3D, 0xFE06341D, 0x044F7095, 0x9FF240E4, 0xF6B67DE5,
                         0xD4699245, u"Player behind wall");
    enabled_changed(&player, *player.primary); // 0x43407C86
    auto& teammate = group(1, u"Teammate", 0x1570AF7C, 0x2CEFC91C, 0xE85380FA, 0xF3D8F0D1,
                           0x57BF65EC, 0x04AD8938, u"Teammate behind wall");
    enabled_changed(&teammate, *teammate.primary); // 0x433FB6B1
    auto& local = group(2, u"Local player", 0x6670DB4D, 0x9201E7ED, 0x3B28B0E5, 0x9436ECB4);
    enabled_changed(&local, *local.primary); // 0x4337CACF
    auto& transparency =
        add<MultiSelect>(category_, 0x3EA36C3A, u"Local player transparency",
                         value<IntegerConfig>(settings, 0x3EA36C3A), identities, false);
    transparency.add_option("Grenade", 1);
    transparency.add_option("Scoped", 2);
    auto& fake = group(3, u"Local player fake", 0x96DB2E1C, 0xA98CEAE8, 0x3FC208C1, 0xF50A4300);
    enabled_changed(&fake, *fake.primary); // 0x433AB16B
    auto& on_shot = group(4, u"On shot", 0x5E0E2558, 0xC2DBF0A4, 0x17C5B91A, 0x8B122392);
    on_shot.duration =
        &add<Slider>(category_, 0x1A07B48F, u"", 2, 100, value<IntegerConfig>(settings, 0x1A07B48F),
                     true, 0x73u, 0.1f, identities);
    enabled_changed(&on_shot, *on_shot.primary); // 0x433CE8BA
    checkbox(0x8CCAF8DC, u"Ragdolls");
    auto& hands = group(5, u"Hands", 0xE8407B64, 0x5A93F490, 0xB7360BA6, 0x2926F5DE);
    enabled_changed(&hands, *hands.primary); // 0x433E520A
    auto& viewmodel = group(6, u"Weapon viewmodel", 0xDA411D58, 0x75A868A4, 0x2A1FA11A, 0x1C678B92);
    enabled_changed(&viewmodel, *viewmodel.primary); // 0x433F9C51
    auto& weapons = group(7, u"Weapons", 0x92EBAD93, 0x83BE3FA7, 0x118A8443, 0x6D66C717);
    enabled_changed(&weapons, *weapons.primary); // 0x433A121B
    checkbox(0x8E0781AE, u"Disable model occlusion");
    checkbox(0x67F8885C, u"Shadow");
    color(0xC2140C28);
    checkbox(0xE217C264, u"Props");
    color(0x84830790);
    tab.categories.push_back(&category_);
}
ColoredModelsMenu::~ColoredModelsMenu() {
    std::erase(tab_.categories, &category_);
}

bool ColoredModelsMenu::active(const MaterialGroup& group) noexcept {
    return (group.primary->value && *group.primary->value != 0) ||
           (group.behind_wall && group.behind_wall->value && *group.behind_wall->value != 0);
}
void ColoredModelsMenu::material_changed(void* context, Widget&) {
    // Equivalent owner-specialized bodies: 0x433ED78B / 0x433A0023 /
    // 0x433AFD35 / 0x43322804 / 0x43329AD5 / 0x4335933C /
    // 0x433593AF / 0x433D6599. Test the values, independent of widget visibility.
    auto& group = *static_cast<MaterialGroup*>(context);
    const auto material = group.material->value ? *group.material->value : 0;
    group.secondary_color->set_visible(active(group) && material >= 2 && material <= 5);
}
void ColoredModelsMenu::enabled_changed(void* context, Widget& source) {
    auto& group = *static_cast<MaterialGroup*>(context);
    const auto enabled = active(group);
    group.material->set_visible(enabled);
    material_changed(context, source);
    if (group.duration)
        group.duration->set_visible(enabled); // 0x433CE8EF..0x433CE8FC
}

} // namespace skeet::ui
