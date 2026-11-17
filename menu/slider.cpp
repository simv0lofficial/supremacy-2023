// Menu Component: slider
#include "skeet/gui_layout.hpp"
#include <bit>
#include <limits>
#include <stdexcept>
#include "skeet/menu/rage_menu.hpp"
#include <immintrin.h>


// ============================================================================
// Subcomponent: scrollbar.cpp
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
i32 divide(i32 a, i32 b) {
    if (!b || (a == std::numeric_limits<i32>::min() && b == -1))
        throw std::overflow_error("Invalid scrollbar integer division");
    return a / b;
}
}
Rectangle scrollbar_thumb(const Scrollbar& scroll, i32 width, float scale) { // 0x433C8760
    if (!scroll.content_height)
        return {};
    const auto scaled_width = round_layout_coordinate(float(width) * scale);
    auto height = divide(mul(scroll.viewport.y, scroll.viewport.y), scroll.content_height);
    if (height < 20)
        height = 20;
    Rectangle thumb{sub(scroll.viewport.x, scaled_width), 2, sub(scaled_width, 2), height};
    const auto travel = sub(sub(scroll.viewport.y, height), 4);
    const auto extent = sub(scroll.content_height, scroll.viewport.y);
    if (travel >= 0 && extent > 0)
        thumb.y = add(divide(mul(scroll.position, travel), extent), 2);
    return thumb;
}
bool scrollbar_mouse(Scrollbar& scroll, std::uint32_t message, Point cursor,
                     float scale) { // 0x4340B88B
    if (message == 0x200) {
        if (!scroll.dragging)
            return false;
        const auto thumb = scrollbar_thumb(scroll, 14, scale);
        const auto y = sub(cursor.y, scroll.grab_offset);
        if (y < 1)
            scroll.position = 0;
        else if (add(add(thumb.height, 1), y) > scroll.viewport.y)
            scroll.position = sub(scroll.content_height, scroll.viewport.y);
        else if (scroll.viewport.y > thumb.height)
            scroll.position = divide(mul(sub(scroll.content_height, scroll.viewport.y), sub(y, 1)),
                                     sub(scroll.viewport.y, thumb.height));
        return true;
    }
    if (message == 0x201) {
        const auto thumb = scrollbar_thumb(scroll, 14, scale);
        if (contains(thumb, cursor)) {
            scroll.dragging = true;
            scroll.grab_offset = sub(cursor.y, thumb.y);
        } else {
            const auto width = round_layout_coordinate(scale * 14.0f);
            if (!contains(
                    {sub(sub(scroll.viewport.x, width), 1), 1, width, sub(scroll.viewport.y, 2)},
                    cursor))
                return false;
            const auto delta =
                cursor.y < thumb.y ? sub(1, scroll.viewport.y) : sub(scroll.viewport.y, 1);
            move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, delta);
        }
        return true;
    }
    if (message == 0x202)
        scroll.dragging = false;
    return false;
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: rage_damage_slider.cpp
// ============================================================================

namespace skeet::ui {
/* [dedup arithmetic helpers] */
RageDamageSlider::RageDamageSlider(Widget* owner, std::u16string_view label, std::int32_t* setting,
                                   WidgetIdentities& identities)
    : Slider(owner, label, 0, 126, setting, true, 0, 1.0f, identities) {
    value_labels.emplace(0, L"Auto");
    if (value && *value == 0)
        update_value_text();
}
void RageDamageSlider::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const {
    if (!value || *value < 101) {
        Slider::draw(queue, origin, accent);
        return;
    }
    if (!parent)
        throw std::logic_error("Slider rendering requires an owning category");
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    const auto current = *value;
    draw_label(queue, position, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 8));
    const auto top = add(position.y, 2), range = sub(maximum, minimum);
    const auto magnitude = range < 0 ? sub(0, range) : range;
    if (geometry.bounds.width < magnitude) {
        if (current != minimum)
            queue.rectangle({sub(position.x, 5), add(top, 3), 3, 1, 0xFF646464});
        if (current != maximum) {
            queue.rectangle(
                {add(add(position.x, geometry.bounds.width), 4), add(top, 3), 3, 1, 0xFF646464});
            queue.rectangle(
                {add(add(position.x, geometry.bounds.width), 5), add(top, 2), 1, 3, 0xFF646464});
        }
    }
    queue.rectangle_outline(
        {position.x, top, geometry.bounds.width, add(track_size.y, 1), 0xFF0C0C0C}, 1);
    const auto upper = !enabled ? 0xFF141414U : hovered ? 0xFF3C3C3CU : 0xFF323232U;
    const auto lower = !enabled ? 0xFF282828U : hovered ? 0xFF505050U : 0xFF464646U;
    const auto x = add(position.x, 1), y = add(top, 1), height = sub(track_size.y, 1);
    queue.vertical_gradient({x, y, sub(geometry.bounds.width, 2), height, upper, lower});
    const auto proportion =
        (1.0f / (float(range) / float(geometry.bounds.width))) * float(sub(current, minimum));
    const auto fill = _mm_cvttss_si32(_mm_set_ss(proportion));
    if (!enabled)
        accent = (accent & 0xFFFFFF) | (((accent >> 24) / 3) << 24);
    queue.vertical_gradient({x, y, fill, height, accent, subtract_color(accent, 0x003C3C3C)});
    std::array<wchar_t, 64> text{};
    const auto length = swprintf_s(text.data(), text.size(), L"HP+%d", sub(current, 100));
    if (length < 0)
        throw std::runtime_error("Damage slider formatting failed");
    queue.text(add(position.x, fill), add(top, 6), enabled ? 0xBBFFFFFF : 0x50FFFFFF, 0x31,
               geometry.bounds.width,
               {reinterpret_cast<const char16_t*>(text.data()), std::size_t(length)});
}
} // namespace skeet::ui
