// Menu Component: multidropdown
#include "skeet/multiselect.hpp"
#include "skeet/font_metrics.hpp"
#include "skeet/scrollbar_render.hpp"
#include <windows.h>
#include <algorithm>
#include <bit>
#include <limits>
#include <stdexcept>


// ============================================================================
// Subcomponent: multiselect.cpp
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
i32 mul(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * std::uint32_t(b));
}
i32 divide(i32 a, i32 b) {
    if (!b || (a == std::numeric_limits<i32>::min() && b == -1))
        throw std::overflow_error("Invalid multiselect integer division");
    return a / b;
}
std::u16string decode_option(const std::string& text) { // 0x43396CB3 / 0x4334E6FC
    std::array<wchar_t, 512> output{};
    const auto count = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, output.data(),
                                           static_cast<int>(output.size()));
    if (count < 1)
        return {};
    return {reinterpret_cast<const char16_t*>(output.data()), static_cast<std::size_t>(count - 1)};
}
}

MultiSelect::MultiSelect() {
    kind = 13;
}
MultiSelect::MultiSelect(Widget* owner, std::u16string_view label, std::uint32_t* setting,
                         WidgetIdentities& identities, bool require_one) // 0x433230AD
    : Widget(owner, label, 13, setting != nullptr, identities), value(setting),
      require_at_least_one(require_one) {
    geometry.default_size = {158, name.empty() ? 20 : 35};
    geometry.bounds.width = geometry.default_size.x;
    geometry.bounds.height = geometry.default_size.y;
    options.reserve(32);
    update_scroll();
}
MultiSelect::MultiSelect(Widget* owner, std::u16string_view label, std::int32_t* setting,
                         WidgetIdentities& identities, bool require_one)
    : MultiSelect(owner, label, reinterpret_cast<std::uint32_t*>(setting), identities,
                  require_one) {}
void MultiSelect::add_option(std::string_view label, std::uint32_t mask) { // 0x4337CB10
    if (options.size() >= 32)
        return;
    options.push_back({std::string(label.substr(0, label.find('\0'))), mask});
    refresh_display();
    update_scroll();
}
void MultiSelect::set_value(std::uint32_t requested) { // 0x433F49B3
    if ((value ? *value : 0) == requested)
        return;
    if (value)
        *value = requested;
    value_changed();
}
std::uint32_t MultiSelect::selected_count() const { // 0x433B580E
    const auto current = value ? *value : 0;
    std::uint32_t count = 0;
    for (const auto& option : options)
        count += (option.mask & current) != 0;
    return count;
}
void MultiSelect::refresh_display() { // 0x433FD945
    if (!value)
        return;
    display.clear();
    for (const auto& option : options) {
        if (!(option.mask & *value))
            continue;
        if (!display.empty())
            display.append(", ");
        display.append(option.name);
    }
}
void MultiSelect::value_changed() {
    notify_callbacks();
    refresh_display();
} // 0x433FFD49
void MultiSelect::config_loaded() {
    refresh_display();
} // 0x43359E24
void MultiSelect::reset_input() { // 0x433B5106
    hovered_item = -1;
    hovered = open = held = false;
}
i32 MultiSelect::input_priority() const {
    return open ? 2 : 0;
} // 0x433CDEA6
bool MultiSelect::hit_test(Point cursor) const { // 0x4336A4FB
    auto bounds = geometry.bounds;
    if (open)
        bounds.height = add(
            bounds.height,
            add(1, mul(static_cast<i32>(std::min<std::size_t>(8, options.size())), row_height)));
    if (!name.empty() && !inline_element) {
        bounds.y = add(bounds.y, interaction_offset);
        bounds.height = sub(bounds.height, interaction_offset);
    }
    return contains(bounds, cursor);
}
bool MultiSelect::interact_options(Point cursor, bool commit) { // 0x433B24C6
    auto y = sub(cursor.y, geometry.bounds.y);
    if (!name.empty() && !inline_element)
        y = sub(y, interaction_offset);
    y = sub(y, row_height);
    if (y < 0) {
        hovered_item = -1;
        return false;
    }
    const auto offset = scroll.enabled ? mul(divide(scroll.position, row_height), row_height) : 0;
    const auto index = static_cast<std::uint32_t>(divide(add(offset, y), row_height));
    if (index >= options.size()) {
        hovered_item = -1;
        return false;
    }
    hovered_item = divide(y, row_height);
    if (commit) {
        const auto count = selected_count();
        const auto current = value ? *value : 0;
        const auto mask = options[index].mask;
        if (current & mask) {
            if (require_at_least_one && count <= 1)
                return true;
            set_value(current & ~mask);
        } else
            set_value(current | mask);
    }
    return true;
}
bool MultiSelect::mouse(const InputMessage& message, Point cursor) { // 0x433A1483
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
        interact_options(cursor, false);
        return held;
    }
    if (message.message == 0x201) {
        if (!hit_test(cursor))
            return false;
        held = true;
        return true;
    }
    if (message.message == 0x202) {
        if (!held)
            return false;
        if (hit_test(cursor)) {
            if (!open)
                open = true;
            else if (!interact_options(cursor, true))
                open = false;
            held = false;
            return true;
        }
        held = false;
        return false;
    }
    if (message.message == 0x204 || message.message == 0x205)
        return hit_test(cursor);
    if (message.message == 0x20a && open && scroll.enabled && hit_test(cursor)) {
        const auto wheel =
            std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(message.parameter >> 16));
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height,
                             mul(row_height, wheel > 0 ? -4 : 4));
        return true;
    }
    return false;
}
void MultiSelect::update_scroll() noexcept { // 0x43383F1A
    if (options.size() < 9) {
        scroll.enabled = false;
        return;
    }
    scroll.enabled = true;
    scroll.viewport = {geometry.bounds.width, mul(row_height, 8)};
    scroll.content_height = mul(static_cast<i32>(options.size()), row_height);
    move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, 0);
}
void MultiSelect::resize(const SliderMetrics& metrics) { // 0x43383731
    const auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    geometry.bounds.width = width < metrics.minimum_width
                                ? metrics.minimum_width
                                : std::min(width, metrics.preferred_maximum_width);
    update_scroll();
}
void MultiSelect::dpi_changed(float factor, const SliderMetrics&) { // 0x433F0B84
    scale = factor;
    resize_element(geometry, factor);
    row_height = round_layout_coordinate(20.0f * factor);
    interaction_offset = round_layout_coordinate(15.0f * factor);
    update_scroll();
}
void MultiSelect::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x433B3A48
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    draw_label(queue, position, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 8));
    const auto text_color = enabled ? 0x88ffffffU : 0x32ffffffU;
    const bool highlight = hovered || open;
    const auto first = highlight ? 0xff292929U : enabled ? 0xff1f1f1fU : 0x501f1f1fU;
    const auto second = highlight ? 0xff2e2e2eU : enabled ? 0xff242424U : 0x50242424U;
    queue.vertical_gradient({add(position.x, 1), add(position.y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), first, second});
    queue.rectangle_outline({position.x, position.y, geometry.bounds.width, row_height, 0xff0c0c0c},
                            1);
    const auto maximum_width = sub(geometry.bounds.width, round_layout_coordinate(scale * 20.0f));
    if (!queue.fonts)
        throw std::logic_error("Multiselect text requires its native font backend");
    const auto vertical_offset = divide(sub(row_height, font_height(*queue.fonts, 0)), 2);
    const auto text = display.empty() ? std::u16string(u"-") : decode_option(display);
    if (!text.empty())
        queue.text(add(position.x, round_layout_coordinate(scale * 7.0f)),
                   add(position.y, vertical_offset), text_color, 0x40, maximum_width, text);
    draw_down_arrow(
        queue,
        {add(sub(position.x, round_layout_coordinate(scale * 12.0f)), geometry.bounds.width),
         add(position.y, divide(sub(row_height, 4), 2))},
        text_color);
    if (!enabled || !open)
        return;
    const auto bounds_reset = queue.is_bounds_reset();
    const auto depth = queue.get_depth();
    queue.reset_bounds(false);
    queue.set_depth(0.15f);
    const Point popup{position.x, add(position.y, add(row_height, 1))};
    const auto rows = static_cast<std::uint32_t>(std::min<std::size_t>(8, options.size()));
    queue.outlined_rectangle({popup.x, popup.y, geometry.bounds.width,
                              mul(row_height, static_cast<i32>(rows)), 0xff232323},
                             0xff0c0c0c, 1);
    const auto offset = scroll.enabled ? divide(scroll.position, row_height) : 0;
    for (std::uint32_t row = 0; row < rows; ++row) {
        const auto index = row + static_cast<std::uint32_t>(offset);
        if (index >= options.size())
            break;
        const auto row_y = add(popup.y, mul(row_height, static_cast<i32>(row)));
        if (row == static_cast<std::uint32_t>(hovered_item))
            queue.rectangle({add(popup.x, 1), add(row_y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), 0xff191919});
        const auto selected = (options[index].mask & (value ? *value : 0)) != 0;
        const auto label = decode_option(options[index].name);
        if (!label.empty())
            queue.text(add(popup.x, round_layout_coordinate(scale * 7.0f)),
                       add(row_y, vertical_offset), selected ? accent : 0xc8ffffffU,
                       selected ? 1 : 0, maximum_width, label);
    }
    if (scroll.enabled)
        draw_scrollbar(queue, scroll, popup, scale);
    queue.reset_bounds(bounds_reset);
    queue.set_depth(depth);
}
}
