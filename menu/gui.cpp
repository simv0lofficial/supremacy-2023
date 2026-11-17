// Menu Component: gui
#include "skeet/core.hpp"
#include <bit>
#include <stdexcept>
#include "skeet/gui_layout.hpp"
#include <immintrin.h>
#include "skeet/widgets.hpp"
#include <cwchar>
#include <intrin.h>
#include "skeet/menu_tree.hpp"
#include <algorithm>
#include "skeet/scrollbar_render.hpp"
#include "skeet/text_metrics.hpp"
#include "skeet/popup_menu.hpp"
#include "skeet/font_metrics.hpp"


// ============================================================================
// Subcomponent: gui.cpp
// ============================================================================

namespace skeet {
namespace {
std::int32_t wrapping_sum(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
}

// Original complete function: 0x433EB3B9..0x433EB3E9.
bool contains(const Rectangle& bounds, Point cursor) noexcept {
    return cursor.x >= bounds.x && cursor.x <= wrapping_sum(bounds.x, bounds.width) &&
           cursor.y >= bounds.y && cursor.y <= wrapping_sum(bounds.y, bounds.height);
}

// Original complete function: 0x433B75D3..0x433B760B.
void select_tab(Menu& menu, std::uint32_t requested_tab) {
    if (requested_tab == menu.active_tab || requested_tab >= menu.tabs.size())
        return;
    if (menu.active_tab >= menu.tabs.size())
        throw std::out_of_range("Menu active tab is outside the tab vector");
    auto& previous = menu.tabs[menu.active_tab];
    previous.reset(previous.object);
    // Reset may change the vector. Reload the requested tab after it returns.
    menu.active_tab = requested_tab;
    auto& next = menu.tabs[requested_tab];
    next.activate(next.object);
}
} // namespace skeet

// ============================================================================
// Subcomponent: gui_layout.cpp
// ============================================================================

namespace skeet::ui {
namespace {
using i32 = std::int32_t;
inline i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
inline i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
i32 mul(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * std::uint32_t(b));
}
i32 truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
}

i32 round_layout_coordinate(float value) noexcept { // 0x433CC0D6
    // Preserve CVTSS2SI: MXCSR rounding mode and INT_MIN for invalid conversions.
    return _mm_cvtss_si32(_mm_set_ss(value));
}
Point scale_size(Point size, float scale) noexcept { // 0x43328177
    const auto height = round_layout_coordinate(float(size.y) * scale);
    return {round_layout_coordinate(float(size.x) * scale), height};
}
void resize_element(ElementGeometry& element, float scale) noexcept { // 0x4336EDA0
    const auto size = scale_size(element.default_size, scale);
    element.bounds.width = size.x;
    element.bounds.height = size.y;
    element.label_height = round_layout_coordinate(scale * 14.0f);
    if (element.horizontal_offset != 0)
        element.horizontal_offset = round_layout_coordinate(scale * 20.0f);
}
void move_scroll_position(i32& position, i32 viewport_height, i32 content_height,
                          i32 delta) noexcept { // 0x433E1D26
    auto updated = add(position, delta);
    const auto maximum = sub(content_height, viewport_height);
    if (updated < 0)
        updated = 0;
    else if (maximum < updated)
        updated = maximum;
    position = updated;
}

void layout_category_grid(const MenuGeometry& menu, const TabGeometry& tab) { // 0x43384477
    const auto left = add(add(tab.width, tab.padding), menu.start_padding.x);
    const auto top = add(menu.start_padding.y, tab.padding);
    const auto right = sub(menu.size.x, add(menu.end_padding.x, tab.padding));
    const auto bottom = sub(menu.size.y, add(menu.end_padding.y, tab.padding));
    std::array<i32, 24> reserved_height{};
    for (auto* category : tab.categories) {
        auto& item = *category;
        if (item.column < 0 || item.column >= 24 || item.column_span < 0 || item.column_span > 24)
            throw std::out_of_range("Category grid indices exceed the native 24-column storage");
        auto& bounds = item.element.bounds;
        auto& defaults = item.element.default_size;
        const auto available_top = add(reserved_height[item.column], top);
        const auto width = float(sub(right, left));
        const auto height = float(sub(bottom, available_top));
        if (item.row_span == -1) {
            bounds.height = defaults.y = add(item.fixed_height, mul(item.margin, 2));
            // The native upper bound is column_span, not column + column_span.
            for (auto column = item.column; column < item.column_span; ++column)
                reserved_height[column] = bounds.height;
        } else {
            bounds.height = defaults.y = truncate((float(item.row_span) / 24.0f) * height);
        }
        bounds.width = defaults.x = truncate((float(item.column_span) / 24.0f) * width);
        bounds.x = add(truncate(float(item.column) * (width / 24.0f) + float(left)),
                       item.element.horizontal_offset);
        bounds.y = truncate((height / 24.0f) * float(item.row) + float(available_top));
        bounds.x = add(bounds.x, item.margin);
        bounds.y = add(bounds.y, item.margin);
        bounds.width = sub(bounds.width, mul(item.margin, 2));
        bounds.height = sub(bounds.height, mul(item.margin, 2));
        const auto final_right = sub(right, item.margin);
        const auto final_bottom = sub(bottom, item.margin);
        if (item.column_span + item.column >= 24 && add(bounds.width, bounds.x) != final_right)
            bounds.width = defaults.x = sub(final_right, bounds.x);
        if (item.row_span + item.row >= 24 && add(bounds.height, bounds.y) != final_bottom)
            bounds.height = defaults.y = sub(final_bottom, bounds.y);
    }
}

void reflow_category(CategoryLayout& category) { // 0x433AD23F
    const auto padding = category.padding;
    const auto initial_height = category.bounds.height;
    auto next_row = padding;
    auto inline_row = padding;
    const auto initial_elements = category.elements;
    for (auto* element : initial_elements) {
        if (!element->visible)
            continue;
        auto& geometry = element->geometry;
        auto& bounds = geometry.bounds;
        if (!element->inline_element) {
            bounds.x = add(geometry.horizontal_offset, padding);
            bounds.y = next_row;
            if (bounds.height > 0) {
                inline_row = next_row;
                next_row = add(add(next_row, 5), bounds.height);
            }
        } else if (element->kind == 12 || element->kind == 9) {
            bounds.x =
                sub(add(sub(geometry.horizontal_offset, bounds.width), category.bounds.width),
                    mul(category.padding, 2));
            bounds.y = add(inline_row, 1);
            inline_row = add(add(inline_row, 5), bounds.height);
        } else {
            bounds.x = add(geometry.horizontal_offset, padding);
            bounds.y = inline_row;
        }
        element->layout(*element);
    }
    auto& scroll = category.scroll;
    if (category.elements.size() < 2 || next_row <= sub(initial_height, 5)) {
        scroll.position = 0;
        scroll.enabled = false;
    } else {
        scroll.enabled = true;
        scroll.viewport = {category.bounds.width, category.bounds.height};
        scroll.content_height = add(category.padding, next_row);
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, 0);
    }
    const auto offset = scroll.enabled ? scroll.position : 0;
    const auto visible_bottom = add(category.bounds.height, offset);
    const auto final_elements = category.elements;
    for (auto* element : final_elements) {
        const auto bottom = add(element->geometry.bounds.height, element->geometry.bounds.y);
        if (bottom < offset || visible_bottom < bottom)
            element->reset(*element);
    }
}

void update_listbox_scroll(ListboxLayout& listbox) noexcept { // 0x4333FF11
    if (listbox.visible_rows >= listbox.item_count) {
        listbox.scroll.enabled = false;
        return;
    }
    auto& scroll = listbox.scroll;
    scroll.enabled = true;
    scroll.viewport.x = listbox.element.bounds.width;
    scroll.viewport.y = listbox.search_visible
                            ? sub(listbox.element.bounds.height, listbox.row_height)
                            : listbox.element.bounds.height;
    scroll.content_height = mul(add(listbox.item_count, 1), listbox.row_height);
    move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, 0);
}
void reflow_listbox(ListboxLayout& listbox, float scale) { // 0x4339E4DF
    resize_element(listbox.element, scale);
    const auto row_height = round_layout_coordinate(scale * 18.0f);
    if (row_height == 0 || (row_height == -1 && listbox.element.bounds.height == INT32_MIN))
        throw std::domain_error("Listbox row height would cause an x86 division fault");
    const auto rows = listbox.element.bounds.height / row_height;
    listbox.row_height = row_height;
    listbox.visible_rows = listbox.search_visible ? sub(rows, 1) : rows;
    update_listbox_scroll(listbox);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: widgets.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup] */
}

Widget::Widget() {
    context = this;
    layout = [](LayoutElement& element) { static_cast<Widget*>(element.context)->on_layout(); };
    reset = [](LayoutElement& element) { static_cast<Widget*>(element.context)->reset_input(); };
}
Widget::Widget(Widget* owner, std::u16string_view label, std::uint8_t type, bool has_value,
               WidgetIdentities& identities)
    : Widget() { // 0x43383D2B
    parent = owner;
    enabled = has_value;
    kind = type;
    name = label;
    geometry.horizontal_offset = 20;
    geometry.label_height = 14;
    identity = identities.allocate();
}
void Widget::reset_input() {
    hovered = false;
} // 0x433E2E4E
bool Widget::hit_test(Point cursor) const {
    return contains(geometry.bounds, cursor);
}
bool Widget::mouse(const InputMessage&, Point) {
    return false;
} // 0x43382057
bool Widget::keyboard(const InputMessage&) {
    return false;
} // 0x43336939
void Widget::config_loaded() {} // 0x433CCE8E
std::int32_t Widget::input_priority() const {
    return 0;
} // 0x43336942
void Widget::resize(const SliderMetrics&) {} // 0x433CCE8E
void Widget::dpi_changed(float scale, const SliderMetrics&) {
    resize_element(geometry, scale);
}
void Widget::draw(RenderQueue&, Point, std::uint32_t) const {} // 0x4334A269
void Widget::notify_callbacks() { // 0x4334E09F
    const auto pending = std::span(callbacks);
    for (const auto& callback : pending)
        callback.function(callback.context, *this);
}
void Widget::set_visible(bool requested) { // 0x433E000B
    if (visible != requested) {
        visible = requested;
        if (parent)
            parent->visibility_changed();
    }
}
void Widget::visibility_changed() { // 0x4332EF98
    if (!visible)
        reset_input();
}
MenuInput* Widget::find_menu() const noexcept { // 0x4334DC50
    for (auto* current = this; current; current = current->parent)
        if (current->kind == 1)
            return current->menu_input;
    return nullptr;
}
void Widget::value_changed() {
    notify_callbacks();
}
void Widget::draw_label(RenderQueue& queue, Point& cursor, i32 maximum_width) const { // 0x43384344
    if (name.empty() || inline_element)
        return;
    auto text = std::u16string_view(name).substr(0, 256);
    if (const auto newline = text.find(u'\n'); newline != text.npos)
        text = text.substr(0, newline);
    if (text.empty())
        return;
    const auto label_color = enabled ? color : (color & 0x00ffffffU) | 0x46000000U;
    queue.text(cursor.x, cursor.y, label_color, text_flags, maximum_width, text);
    cursor.y = add(cursor.y, geometry.label_height);
}

Label::Label() {
    kind = 8;
}
Label::Label(Widget* owner, std::u16string_view text, WidgetIdentities& identities) // 0x43328C61
    : Widget(owner, text, 8, false, identities) {
    enabled = true;
    text_flags = 0;
    geometry.default_size = {200, 12};
    geometry.bounds.width = 200;
    geometry.bounds.height = 12;
}
void Label::draw(RenderQueue& queue, Point origin, std::uint32_t) const { // 0x433F8EB7
    Point cursor{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    draw_label(queue, cursor, 0);
}

Checkbox::Checkbox() {
    kind = 3;
    tied.next = &tied;
    geometry.horizontal_offset = 0;
}
Checkbox::Checkbox(Widget* owner, std::u16string_view label, std::uint8_t* setting,
                   WidgetIdentities& identities) // 0x433ADF35
    : Widget(owner, label, 3, setting != nullptr, identities), value(setting) {
    tied.next = &tied;
    geometry.horizontal_offset = 0;
    geometry.default_size = name.empty() ? Point{17, 0} : Point{150, 13};
    geometry.bounds.width = geometry.default_size.x;
    geometry.bounds.height = geometry.default_size.y;
}
bool Checkbox::hit_test(Point cursor) const { // 0x4339C9E1
    auto bounds = geometry.bounds;
    if (bounds.height == 0)
        bounds.height = 13;
    return contains(bounds, cursor);
}
void Checkbox::set_value(bool checked) { // 0x433AD3C8
    const auto previous = value ? *value : false;
    if (previous != checked) {
        if (value)
            __atomic_exchange_n(value, checked, __ATOMIC_SEQ_CST);
        value_changed();
    }
}
void Checkbox::config_loaded() { // 0x433B927E
    for (auto* link = tied.next; link != &tied; link = link->next)
        link->widget->set_visible(value ? *value : false);
}
void Checkbox::value_changed() { // 0x4334C75F
    notify_callbacks();
    config_loaded();
}
void Checkbox::reset_input() { // 0x4332C42A
    hovered = false;
    held = false;
}
bool Checkbox::mouse(const InputMessage& message, Point cursor) { // 0x4333043A
    if (message.message == 0x200)
        hovered = hit_test(cursor);
    else if (message.message == 0x201) {
        if (hit_test(cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x202 && held) {
        held = false;
        if (hit_test(cursor))
            set_value(!(value && *value));
        return true;
    }
    return false;
}
void Checkbox::dpi_changed(float scale) { // 0x433E085C
    resize_element(geometry, scale);
    box_size = scale_size({8, 8}, scale);
    text_padding = round_layout_coordinate(scale * 20.0f);
}
void Checkbox::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x4338902F
    if (!parent)
        throw std::logic_error("Checkbox rendering requires an owning category");
    auto top_color = value && *value ? accent : 0xbb646464U;
    if (!enabled)
        top_color = (top_color & 0x00ffffffU) | 0x46000000U;
    const Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    queue.rectangle_outline({position.x, add(position.y, 3), box_size.x, box_size.y, 0xff0c0c0cU},
                            1);
    auto bottom_color = subtract_color(top_color, 0x00282828);
    if (hovered) {
        top_color = add_color(top_color, 0x000a0a0a);
        bottom_color = add_color(bottom_color, 0x000a0a0a);
    }
    queue.vertical_gradient({add(position.x, 1), add(position.y, 4), sub(box_size.x, 2),
                             sub(box_size.y, 2), top_color, bottom_color});
    Point label{add(position.x, text_padding), position.y};
    draw_label(queue, label, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 28));
}

Slider::Slider() {
    kind = 5;
}
Slider::Slider(Widget* owner, std::u16string_view label, i32 lower, i32 upper, i32* setting,
               bool show, std::uint32_t unit, float multiplier,
               WidgetIdentities& identities) // 0x433CF6B2
    : Widget(owner, label, 5, setting != nullptr, identities), value(setting), show_value(show),
      maximum(upper), minimum(lower), display_scale(multiplier), suffix(unit) {
    geometry.default_size = {158, name.empty() ? 10 : 25};
    geometry.bounds.width = geometry.default_size.x;
    geometry.bounds.height = geometry.default_size.y;
    update_value_text();
}
bool Slider::hit_test(Point cursor) const { // 0x433BA3F2
    const auto& bounds = geometry.bounds;
    return sub(bounds.x, 10) <= cursor.x && cursor.x <= add(add(bounds.width, 10), bounds.x) &&
           sub(bounds.y, 2) <= cursor.y && cursor.y <= add(add(bounds.height, 2), bounds.y);
}
i32 Slider::value_at(Point cursor) const noexcept { // 0x43365AF6
    const auto& bounds = geometry.bounds;
    const auto position =
        (float(sub(maximum, minimum)) / float(bounds.width)) * float(sub(cursor.x, bounds.x)) +
        float(minimum);
    auto result = _mm_cvttss_si32(_mm_set_ss(position));
    if (result < minimum)
        result = minimum;
    else if (maximum < result)
        result = maximum;
    return result;
}
void Slider::set_value(i32 requested) { // 0x43398ECF
    const auto previous = value ? *value : 0;
    if (previous != requested) {
        if (value)
            _InterlockedExchange(reinterpret_cast<volatile long*>(value), long(requested));
        value_changed();
    }
}
void Slider::value_changed() { // 0x433A1670
    auto current = value ? *value : 0;
    if (current < minimum)
        current = minimum;
    else if (maximum < current)
        current = maximum;
    if (value)
        _InterlockedExchange(reinterpret_cast<volatile long*>(value), long(current));
    notify_callbacks();
    update_value_text();
}
void Slider::config_loaded() { // 0x4341B3DD
    if (value) {
        if (*value < minimum)
            set_value(minimum);
        else if (*value > maximum)
            set_value(maximum);
    }
    update_value_text();
}
void Slider::update_value_text() { // 0x4334A31C
    static_assert(sizeof(wchar_t) == 2);
    value_text.fill(0);
    if (!show_value || !value)
        return;
    const auto current = *value;
    if (auto label = value_labels.find(std::uint32_t(current)); label != value_labels.end()) {
        if (label->second.size() >= value_text.size())
            throw std::length_error("Slider value label exceeds the native UTF-16 buffer");
        std::copy(label->second.begin(), label->second.end(), value_text.begin());
        return;
    }
    int length;
    if (display_scale == 1.0f) {
        length = swprintf_s(value_text.data(), value_text.size(), L"%d", current);
    } else if (display_scale > 1.0f) {
        const auto scaled = _mm_cvttss_si32(_mm_set_ss(float(current) * display_scale));
        length = swprintf_s(value_text.data(), value_text.size(), L"%d", scaled);
    } else {
        const auto scaled = float(current) * display_scale;
        length = swprintf_s(value_text.data(), value_text.size(),
                            display_scale == 0.001f ? L"%.3f" : L"%.2f", double(scaled));
        while (length > 1 && value_text[length - 2] != L'.' && value_text[length - 1] == L'0')
            value_text[--length] = 0;
    }
    if (length < 0)
        throw std::runtime_error("Slider number formatting failed");
    if (suffix != 0) {
        if (std::size_t(length) + 2 >= value_text.size())
            throw std::length_error("Slider suffix exceeds the native UTF-16 buffer");
        value_text[length] = wchar_t(suffix & 0xffff);
        value_text[length + 1] = wchar_t(suffix >> 16);
        value_text[length + 2] = 0;
    }
}
bool Slider::mouse(const InputMessage& message, Point cursor) { // 0x433E5250
    if (message.message == 0x200) {
        hovered = hit_test(cursor);
        if (!dragging)
            return false;
    } else if (message.message == 0x201) {
        if (!hit_test(cursor))
            return false;
        if (cursor.x < geometry.bounds.x) {
            set_value(sub(value ? *value : 0, 1));
            return true;
        }
        if (add(geometry.bounds.x, geometry.bounds.width) < cursor.x) {
            set_value(add(value ? *value : 0, 1));
            return true;
        }
        dragging = true;
    } else {
        if (message.message == 0x202)
            dragging = false;
        else if (message.message == 0x20a && hit_test(cursor)) {
            auto* menu = find_menu();
            if (!menu)
                throw std::logic_error("Slider wheel input requires an owning menu");
            if (menu->button_flags & 8) {
                const auto wheel =
                    std::bit_cast<std::int16_t>(std::uint16_t(message.parameter >> 16));
                set_value(add(value ? *value : 0, wheel >= 1 ? 1 : -1));
                return true;
            }
        }
        return false;
    }
    set_value(value_at(cursor));
    return true;
}
bool Slider::keyboard(const InputMessage& message) { // 0x4336BC61
    if (message.message != 0x100 || !value)
        return false;
    const auto* menu = find_menu();
    const auto step = menu && (menu->button_flags & 4) ? 5 : 1;
    if (message.parameter == 0x25)
        *value = sub(*value, step);
    else if (message.parameter == 0x27)
        *value = add(*value, step);
    else
        return false;
    value_changed();
    return true;
}
void Slider::reset_input() { // 0x433689A4
    hovered = false;
    dragging = false;
}
void Slider::resize(const SliderMetrics& metrics) { // 0x433F0A8F
    if (!parent)
        throw std::logic_error("Slider resizing requires an owning category");
    auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    if (width < metrics.minimum_width)
        width = metrics.minimum_width;
    if (maximum_width != 0 && maximum_width < width)
        width = maximum_width;
    geometry.bounds.width = width;
}
void Slider::dpi_changed(float scale, const SliderMetrics& metrics) { // 0x4340BFC4
    resize_element(geometry, scale);
    track_size = scale_size({158, 6}, scale);
    if (maximum_width != 0)
        maximum_width = metrics.preferred_maximum_width;
}
void Slider::draw_value(RenderQueue& queue, i32 x, i32 y, i32 fill_width, i32) const { // 0x43416FBD
    if (!show_value || value_text[0] == 0)
        return;
    const auto text = std::bit_cast<std::array<char16_t, 64>>(value_text);
    const auto length = std::char_traits<char16_t>::length(text.data());
    queue.text(add(x, fill_width), add(y, track_size.y), enabled ? 0xbbffffffU : 0x50ffffffU, 0x31,
               geometry.bounds.width, {text.data(), length});
}
void Slider::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x43383A3C
    if (!parent)
        throw std::logic_error("Slider rendering requires an owning category");
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    const auto current = value ? *value : 0;
    draw_label(queue, position, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 8));
    const auto top = add(position.y, 2);
    const auto range = sub(maximum, minimum);
    const auto magnitude = range < 0 ? sub(0, range) : range;
    if (geometry.bounds.width < magnitude) {
        if (current != minimum)
            queue.rectangle({sub(position.x, 5), add(top, 3), 3, 1, 0xff646464U});
        if (current != maximum) {
            queue.rectangle(
                {add(add(position.x, geometry.bounds.width), 4), add(top, 3), 3, 1, 0xff646464U});
            queue.rectangle(
                {add(add(position.x, geometry.bounds.width), 5), add(top, 2), 1, 3, 0xff646464U});
        }
    }
    queue.rectangle_outline(
        {position.x, top, geometry.bounds.width, add(track_size.y, 1), 0xff0c0c0cU}, 1);
    const auto upper = !enabled ? 0xff141414U : hovered ? 0xff3c3c3cU : 0xff323232U;
    const auto lower = !enabled ? 0xff282828U : hovered ? 0xff505050U : 0xff464646U;
    const auto inner_x = add(position.x, 1), inner_y = add(top, 1), height = sub(track_size.y, 1);
    queue.vertical_gradient(
        {inner_x, inner_y, sub(geometry.bounds.width, 2), height, upper, lower});
    const auto proportion =
        (1.0f / (float(range) / float(geometry.bounds.width))) * float(sub(current, minimum));
    const auto fill_width = _mm_cvttss_si32(_mm_set_ss(proportion));
    if (!enabled)
        accent = (accent & 0x00ffffffU) | (((accent >> 24) / 3) << 24);
    queue.vertical_gradient(
        {inner_x, inner_y, fill_width, height, accent, subtract_color(accent, 0x003c3c3c)});
    draw_value(queue, position.x, top, fill_width, height);
}

} // namespace skeet::ui

// ============================================================================
// Subcomponent: menu_tree.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup] */
i32 magnitude(i32 a) noexcept {
    return a < 0 ? sub(0, a) : a;
}
std::int8_t narrow(i32 a) noexcept {
    return std::bit_cast<std::int8_t>(std::uint8_t(a));
}
void merge_input(std::span<Widget*> left, std::span<Widget*> right, Widget** output) { // 0x433E8B57
    std::size_t l = 0, r = 0;
    while (l < left.size() && r < right.size()) {
        const auto right_priority = right[r]->input_priority();
        const auto left_priority = left[l]->input_priority(); // 0x4341F30E
        *output++ = right_priority > left_priority ? right[r++] : left[l++];
    }
    while (l < left.size())
        *output++ = left[l++];
    while (r < right.size())
        *output++ = right[r++];
}
void sort_input(std::span<Widget*> input, std::span<Widget*> scratch) { // 0x433ED024
    if (input.size() <= 1)
        return;
    const auto middle = input.size() / 2;
    auto prepare = [&](std::span<Widget*> part, std::span<Widget*> temporary) {
        if (part.size() == 1) {
            temporary[0] = part[0];
            return;
        }
        const auto half = part.size() / 2;
        sort_input(part.first(half), temporary.first(half));
        sort_input(part.subspan(half), temporary.subspan(half));
        merge_input(part.first(half), part.subspan(half), temporary.data());
    };
    prepare(input.first(middle), scratch.first(middle));
    prepare(input.subspan(middle), scratch.subspan(middle));
    merge_input(scratch.first(middle), scratch.subspan(middle), input.data());
}
MenuInput& menu_state(Widget& widget) {
    auto* state = widget.find_menu();
    if (!state)
        throw std::logic_error("Category input requires its menu context");
    return *state;
}
}
Category::Category(Widget* owner, std::u16string_view label, GridPosition position,
                   std::size_t reserve_elements, bool allow_resize,
                   WidgetIdentities& identities) // 0x4348210C / 0x4348E543
    : Widget(owner, label, 7, false, identities), column(position.column),
      column_span(position.column_span), row(position.row), row_span(position.row_span),
      resizable(allow_resize), movable(position.row_span != -1) {
    geometry.horizontal_offset = 0;
    text_flags = 9;
    if (reserve_elements)
        elements.reserve(reserve_elements);
}
void Category::reset_children() { // 0x433E4282
    drag_state = 0;
    for (auto* child : std::span(elements))
        child->reset_input();
}
void Category::reset_input() { // 0x433CDFD7
    background = 0xff171717;
    reset_children();
}
bool Category::keyboard(const InputMessage& message) { // 0x433FBE32
    for (auto* child : std::span(elements))
        if (child->enabled && child->visible && child->keyboard(message))
            return true;
    return false;
}
void Category::config_loaded() { // 0x43402B5C
    for (auto*& child : std::span(elements)) {
        const auto original_kind = child->kind;
        if (original_kind == 2)
            continue;
        child->config_loaded();
        if (original_kind != 9)
            child->value_changed();
    }
}
std::vector<Widget*> Category::input_order() const { // 0x433C8238
    const auto count = std::min(elements.size(), std::size_t(1024));
    std::vector<Widget*> ordered(elements.begin(), elements.begin() + count);
    std::vector<Widget*> temporary(count);
    sort_input(ordered, temporary);
    return ordered;
}
void Category::reflow() { // 0x433AD23F, typed owning-container entry
    const auto initial_height = geometry.bounds.height;
    auto next_row = padding, inline_row = padding;
    for (auto* child : std::span(elements)) {
        if (!child->visible)
            continue;
        auto& bounds = child->geometry.bounds;
        if (!child->inline_element) {
            bounds.x = add(child->geometry.horizontal_offset, padding);
            bounds.y = next_row;
            if (bounds.height > 0) {
                inline_row = next_row;
                next_row = add(add(next_row, 5), bounds.height);
            }
        } else if (child->kind == 12 || child->kind == 9) {
            bounds.x = sub(
                add(sub(child->geometry.horizontal_offset, bounds.width), geometry.bounds.width),
                add(padding, padding));
            bounds.y = add(inline_row, 1);
            inline_row = add(add(inline_row, 5), bounds.height);
        } else {
            bounds.x = add(child->geometry.horizontal_offset, padding);
            bounds.y = inline_row;
        }
        child->on_layout();
    }
    if (elements.size() < 2 || next_row <= sub(initial_height, 5)) {
        scroll.position = 0;
        scroll.enabled = false;
    } else {
        scroll.enabled = true;
        scroll.viewport = {geometry.bounds.width, geometry.bounds.height};
        scroll.content_height = add(padding, next_row);
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, 0);
    }
    const auto offset = scroll.enabled ? scroll.position : 0;
    const auto bottom = add(geometry.bounds.height, offset);
    for (auto* child : std::span(elements)) {
        const auto child_bottom = add(child->geometry.bounds.y, child->geometry.bounds.height);
        if (child_bottom < offset || bottom < child_bottom)
            child->reset_input();
    }
}
void Category::visibility_changed() { // 0x433D08F1
    if (!visible)
        reset_input();
    reflow();
}
void Category::resize(const SliderMetrics& metrics) { // 0x4341B3A1
    for (auto* child : std::span(elements))
        child->resize(metrics);
}
void Category::dpi_changed(float scale, const SliderMetrics& metrics) { // 0x43388E3F
    resize_element(geometry, scale);
    padding = round_layout_coordinate(scale * 20.0f);
    margin = round_layout_coordinate(scale * 10.0f);
    fixed_height = round_layout_coordinate(float(default_fixed_height) * scale);
    for (auto* child : std::span(elements))
        child->dpi_changed(scale, metrics);
    reflow();
}
bool Category::move(Point cursor) { // 0x4334C2C3
    if (!movable)
        return false;
    auto& tab = *static_cast<Tab*>(parent);
    if (tab.focused_widget() != this)
        return false;
    const auto dx = sub(cursor.x, drag_origin.x), dy = sub(cursor.y, drag_origin.y);
    auto next_column = column, next_row = row;
    if (magnitude(dx) >= sub(tab.menu_geometry.size.x, 105) / 24) {
        next_column = narrow(column + (dx > 0 ? 1 : -1));
        if (next_column < 0)
            next_column = 0;
        else if (next_column > narrow(24 - column_span))
            next_column = narrow(24 - column_span);
    }
    if (magnitude(dy) >= sub(tab.menu_geometry.size.y, 35) / 24) {
        next_row = narrow(row + (dy > 0 ? 1 : -1));
        if (next_row < 0)
            next_row = 0;
        else if (next_row > narrow(24 - row_span))
            next_row = narrow(24 - row_span);
    }
    if (next_column != column || next_row != row) {
        Category* collided = nullptr;
        for (auto* other : std::span(tab.categories)) {
            if (other == this)
                continue;
            if (std::max(i32(next_column), i32(other->column)) <
                    std::min(next_column + column_span, other->column + other->column_span) &&
                std::max(i32(next_row), i32(other->row)) <
                    std::min(next_row + row_span, other->row + other->row_span)) {
                collided = other;
                break;
            }
        }
        if (collided) {
            std::swap(column, collided->column);
            std::swap(column_span, collided->column_span);
            std::swap(row, collided->row);
            std::swap(row_span, collided->row_span);
        } else {
            column = next_column;
            row = next_row;
        }
        drag_origin = cursor;
        tab.layout_categories();
        tab.reflow_categories();
    }
    return true;
}
bool Category::change_size(Point cursor) { // 0x4341D248
    if (!resizable)
        return false;
    auto& tab = *static_cast<Tab*>(parent);
    if (tab.focused_widget() != this)
        return false;
    const auto dx = sub(cursor.x, drag_origin.x), dy = sub(cursor.y, drag_origin.y);
    bool changed = false;
    if (magnitude(dx) >= sub(tab.menu_geometry.size.x, 105) / 24) {
        auto next = narrow(column_span + (dx > 0 ? 1 : -1));
        if (next < 3)
            next = 3;
        else if (next > narrow(24 - column))
            next = narrow(24 - column);
        if (next != column_span) {
            column_span = next;
            drag_origin.x = cursor.x;
            changed = true;
        }
    }
    if (magnitude(dy) >= sub(tab.menu_geometry.size.y, 35) / 24) {
        auto next = narrow(row_span + (dy > 0 ? 1 : -1));
        if (next < 2)
            next = 2;
        else if (next > narrow(24 - row))
            next = narrow(24 - row);
        if (next != row_span) {
            row_span = next;
            drag_origin.y = cursor.y;
            changed = true;
        }
    }
    if (changed)
        tab.resize(menu_state(*this).slider_metrics);
    return true;
}
bool Category::mouse(const InputMessage& message, Point cursor) { // 0x4333F951
    if (elements.empty())
        return false;
    const Point local{sub(cursor.x, geometry.bounds.x), sub(cursor.y, geometry.bounds.y)};
    Point content = local;
    bool outside = false;
    if (scroll.enabled) {
        outside = local.x < 5 || local.x > sub(geometry.bounds.width, 10) || local.y < 20 ||
                  local.y > sub(geometry.bounds.height, 10);
        if (scroll.position)
            content.y = add(content.y, scroll.position);
    }
    if (message.message == 0x202)
        drag_state = 0;
    for (auto* child : input_order()) {
        if (child->enabled && child->visible && (!outside || child->input_priority() > 0) &&
            child->mouse(message, content)) {
            child->request_focus();
            return true;
        }
    }
    auto& state = menu_state(*this);
    if (message.message == 0x201 && hit_test(cursor)) {
        drag_origin = cursor;
        if (!state.lock_category_layout && resizable) {
            const auto threshold = state.scale * 15.0f;
            const auto pixels = _mm_cvttss_si32(_mm_set_ss(threshold));
            if (local.x > sub(geometry.bounds.width, pixels) &&
                local.y > sub(geometry.bounds.height, pixels))
                drag_state = 2;
            else if (threshold > float(local.y))
                drag_state = 1;
            else
                goto scroll_input;
            request_focus();
            return true;
        }
    } else if (message.message == 0x200 && !state.lock_category_layout && resizable) {
        if (drag_state == 1 && move(cursor))
            return true;
        if (drag_state == 2 && change_size(cursor))
            return true;
    }
scroll_input:
    if (!scroll.enabled)
        return false;
    if (!scrollbar_mouse(scroll, message.message, local, state.scale)) {
        if (message.message != 0x20a || !hit_test(cursor))
            return false;
        auto delta = round_layout_coordinate(state.scale * 20.0f);
        if (std::bit_cast<std::int16_t>(std::uint16_t(message.parameter >> 16)) > 0)
            delta = sub(0, delta);
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, delta);
    }
    for (auto* child : std::span(elements))
        child->reset_input();
    return true;
}
Widget* Tab::focused_widget() noexcept { // 0x4336EAD1
    if (focus.identity == UINT32_MAX)
        return nullptr;
    if (focus.category == UINT32_MAX)
        return identity == focus.identity ? this : nullptr;
    if (focus.category >= categories.size())
        return nullptr;
    auto* category = categories[focus.category];
    Widget* candidate = category;
    if (focus.element != UINT32_MAX) {
        if (focus.element >= category->elements.size())
            return nullptr;
        candidate = category->elements[focus.element];
    }
    return candidate->identity == focus.identity ? candidate : nullptr;
}
Tab::Tab(Menu& menu, MenuInput& input, MenuGeometry& layout, std::u16string_view label,
         Point& next_position, WidgetIdentities& identities) // 0x4348E41D
    : Widget(nullptr, label, 1, false, identities), navigation(&menu), menu_geometry(layout) {
    menu_input = &input;
    geometry.horizontal_offset = 0;
    geometry.bounds = {next_position.x, next_position.y, 75, 64};
    geometry.default_size = {75, 64};
    next_position.y = add(next_position.y, 64);
}
FocusPath Tab::path_to(const Widget* target) const noexcept { // 0x4332AA08
    FocusPath path;
    if (!target)
        return path;
    if (target == this) {
        path.identity = target->identity;
        return path;
    }
    for (std::uint32_t category_index = 0; category_index < categories.size(); ++category_index) {
        const auto* category = categories[category_index];
        if (target == category)
            return {category_index, UINT32_MAX, target->identity};
        for (std::uint32_t index = 0; index < category->elements.size(); ++index)
            if (category->elements[index] == target)
                return {category_index, index, target->identity};
    }
    return path;
}
void Tab::focus_widget(Widget* target) { // 0x433B1444
    auto* previous = focused_widget();
    if (!target || previous != target) {
        if (previous)
            previous->reset_input();
        focus = path_to(target);
    }
}
void Tab::reset_input() { // 0x43340044
    if (auto* previous = focused_widget(); previous && previous->kind == 10)
        focus_widget(nullptr);
    hovered = false;
    for (auto* category : std::span(categories))
        category->reset_input();
}
bool Tab::keyboard(const InputMessage& message) { // 0x433E9E96
    for (auto* category : std::span(categories))
        if (category->keyboard(message))
            return true;
    return false;
}
void Tab::config_loaded() { // 0x4334C8AC
    for (auto* category : std::span(categories))
        category->config_loaded();
}
bool Tab::mouse(const InputMessage& message, Point cursor) { // 0x433ECEA1
    if (message.message == 0x201 || message.message == 0x200) {
        if (hit_test(cursor)) {
            if (message.message == 0x201) {
                if (!navigation)
                    throw std::logic_error("Tab selection requires its menu");
                select_tab(*navigation, identity);
                return true;
            }
            hovered = true;
            return false;
        }
        hovered = false;
    }
    if (!navigation)
        throw std::logic_error("Tab input requires its menu");
    if (navigation->active_tab != identity)
        return false;
    auto* focused_category = owning_category(focused_widget());
    if (focused_category && focused_category->visible && focused_category->mouse(message, cursor))
        return true;
    for (auto* category : std::span(categories))
        if (category != focused_category && category->visible && category->mouse(message, cursor))
            return true;
    return false;
}
void Tab::layout_categories() { // 0x43384477, owning-container entry
    std::vector<CategoryGeometry> placement(categories.size());
    std::vector<CategoryGeometry*> pointers(categories.size());
    for (std::size_t i = 0; i < categories.size(); ++i) {
        const auto& c = *categories[i];
        placement[i] = {c.geometry,    c.fixed_height, c.margin,  c.column,
                        c.column_span, c.row,          c.row_span};
        pointers[i] = &placement[i];
    }
    layout_category_grid(menu_geometry, {geometry.bounds.width, padding, pointers});
    for (std::size_t i = 0; i < categories.size(); ++i)
        categories[i]->geometry = placement[i].element;
}
void Tab::reflow_categories() { // 0x433CF58B
    for (auto* category : std::span(categories))
        category->reflow();
}
void Tab::resize(const SliderMetrics& metrics) { // 0x433CCAE7
    layout_categories();
    reflow_categories();
    for (auto* category : std::span(categories))
        category->resize(metrics);
}
void Tab::dpi_changed(float scale, const SliderMetrics& metrics) { // 0x433817D6
    padding = round_layout_coordinate(scale * 10.0f);
    resize_element(geometry, scale);
    for (auto* category : std::span(categories))
        category->dpi_changed(scale, metrics);
}
void Tab::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x433AC635
    if (!navigation)
        throw std::logic_error("Tab rendering requires its menu");
    const bool selected = navigation->active_tab == identity;
    const auto tint = selected ? 0xffffffffU : hovered ? 0xc8ffffffU : 0x64ffffffU;
    const auto& bounds = geometry.bounds;
    const Point position{add(origin.x, bounds.x), add(origin.y, bounds.y)};
    if (icon == UINT32_MAX) {
        if (!name.empty() && add(bounds.y, bounds.height) < menu_geometry.size.y)
            queue.text(add(position.x, bounds.width / 2), add(position.y, bounds.height / 2), tint,
                       0x12, 0, std::u16string_view(name).substr(0, 64));
    } else {
        queue.texture({add(position.x, sub(bounds.width, icon_source.width) / 2),
                       add(position.y, sub(bounds.height, icon_source.height) / 2), icon, tint,
                       icon_source.x, icon_source.y, icon_source.width, icon_source.height, 0,
                       1.0f});
    }
    if (selected) {
        auto current = categories.end();
        while (current != categories.begin()) {
            auto* category = *--current;
            if (category->visible)
                category->draw(queue, origin, accent);
        }
    }
}
Category* owning_category(Widget* widget) noexcept { // 0x43364799
    while (widget && widget->kind != 7)
        widget = widget->parent;
    return static_cast<Category*>(widget);
}
Tab* owning_tab(Widget* widget) noexcept { // 0x43330233
    while (widget && widget->kind != 1)
        widget = widget->parent;
    return static_cast<Tab*>(widget);
}
void Widget::request_focus() { // 0x4337ED94
    if (focusable && visible)
        if (auto* tab = owning_tab(this))
            tab->focus_widget(this);
}
} // namespace skeet::ui

// ============================================================================
// Subcomponent: menu_render.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup] */
void merge_draw(std::span<Widget*> left, std::span<Widget*> right,
                Widget** output) { // 0x433E8B57 / 0x4333C986
    std::size_t l = 0, r = 0;
    while (l < left.size() && r < right.size()) {
        const auto right_priority = right[r]->input_priority();
        const auto left_priority = left[l]->input_priority();
        *output++ = right_priority < left_priority ? right[r++] : left[l++];
    }
    while (l < left.size())
        *output++ = left[l++];
    while (r < right.size())
        *output++ = right[r++];
}
void sort_draw(std::span<Widget*> input, std::span<Widget*> scratch) { // 0x433ED024
    if (input.size() <= 1)
        return;
    const auto middle = input.size() / 2;
    auto prepare = [&](std::span<Widget*> part, std::span<Widget*> temporary) {
        if (part.size() == 1) {
            temporary[0] = part[0];
            return;
        }
        const auto half = part.size() / 2;
        sort_draw(part.first(half), temporary.first(half));
        sort_draw(part.subspan(half), temporary.subspan(half));
        merge_draw(part.first(half), part.subspan(half), temporary.data());
    };
    prepare(input.first(middle), scratch.first(middle));
    prepare(input.subspan(middle), scratch.subspan(middle));
    merge_draw(scratch.first(middle), scratch.subspan(middle), input.data());
}
void arrow(RenderQueue& queue, Point point, std::uint32_t color, bool down) {
    const auto step = down ? 1 : -1;
    queue.rectangle({point.x, sub(point.y, step), 5, 1, 0xff000000});
    queue.rectangle({point.x, point.y, 5, 1, color});
    queue.rectangle({add(point.x, 1), add(point.y, step), 3, 1, color});
    queue.rectangle({add(point.x, 2), add(point.y, step * 2), 1, 1, color});
}
}
void draw_up_arrow(RenderQueue& queue, Point point, std::uint32_t color) {
    arrow(queue, point, color, false);
} // 0x43326909
void draw_down_arrow(RenderQueue& queue, Point point, std::uint32_t color) {
    arrow(queue, point, color, true);
} // 0x433C9F33
void draw_scrollbar(RenderQueue& queue, const Scrollbar& scroll, Point origin, float scale,
                    std::uint32_t alpha) { // 0x433E7AFE
    const auto width = round_layout_coordinate(scale * 7.0f);
    const auto track_color = ((alpha & 255) << 24) | 0x00282828U;
    const auto thumb_color = ((alpha & 255) << 24) | 0x00414141U;
    queue.rectangle({sub(add(origin.x, sub(scroll.viewport.x, width)), 1), add(origin.y, 1), width,
                     sub(scroll.viewport.y, 2), track_color});
    const auto thumb = scrollbar_thumb(scroll, 7, scale);
    queue.rectangle(
        {add(origin.x, thumb.x), add(origin.y, thumb.y), thumb.width, thumb.height, thumb_color});
}
std::vector<Widget*> Category::draw_order() const { // 0x433FC71A
    std::vector<Widget*> ordered;
    ordered.reserve(std::min(elements.size(), std::size_t(1024)));
    for (auto* child : elements) {
        if (child->visible) {
            ordered.push_back(child);
            if (ordered.size() == 1024)
                break;
        }
    }
    std::vector<Widget*> temporary(ordered.size());
    sort_draw(ordered, temporary);
    return ordered;
}
void Category::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x433AE90F
    const auto& b = geometry.bounds;
    const Point position{add(origin.x, b.x), add(origin.y, b.y)};
    const auto border = drag_state == 2 ? accent : inner_border;
    const bool titled = show_title && !name.empty();
    std::u16string_view title(name);
    if (titled && title.size() >= 256)
        throw std::length_error("Category title exceeds its native UTF-16 buffer");
    auto metric_title = title;
    if (const auto zero = metric_title.find(char16_t(0)); zero != metric_title.npos)
        metric_title = metric_title.substr(0, zero);
    if (titled) { // 0x4334AF01
        queue.rectangle({add(position.x, 2), add(position.y, 2), sub(b.width, 3), sub(b.height, 3),
                         background});
        queue.rectangle({position.x, position.y, 10, 1, outer_border});
        queue.rectangle({add(position.x, 1), add(position.y, 1), 9, 1, border});
        const auto size = measure_text(queue, metric_title, text_flags);
        const auto right_x = add(add(position.x, 17), size.x),
                   right_width = sub(sub(b.width, size.x), 16);
        queue.rectangle({right_x, position.y, right_width, 1, outer_border});
        queue.rectangle({right_x, add(position.y, 1), right_width, 1, border});
        queue.rectangle({position.x, position.y, 1, b.height, outer_border});
        queue.rectangle({add(position.x, 1), add(position.y, 1), 1, b.height, border});
        queue.rectangle({add(position.x, b.width), position.y, 1, b.height, outer_border});
        queue.rectangle(
            {add(sub(position.x, 1), b.width), add(position.y, 1), 1, b.height, border});
        queue.rectangle({position.x, add(position.y, b.height), add(b.width, 1), 1, outer_border});
        queue.rectangle(
            {add(position.x, 1), add(sub(position.y, 1), b.height), sub(b.width, 1), 1, border});
    } else {
        queue.rectangle_outline({position.x, position.y, b.width, b.height, outer_border}, 1);
        queue.rectangle_outline(
            {add(position.x, 1), add(position.y, 1), sub(b.width, 2), sub(b.height, 2), border}, 1);
        queue.rectangle({add(position.x, 2), add(position.y, 2), sub(b.width, 4), sub(b.height, 4),
                         background});
    }
    if (titled) {
        const auto size = measure_text(queue, metric_title, text_flags);
        queue.text(add(position.x, 14), sub(position.y, size.y / 2),
                   drag_state == 1 ? accent : title_color, text_flags, 0, title);
        if (!scroll.enabled && resizable) {
            const auto x = add(sub(position.x, 1), b.width), y = add(sub(position.y, 1), b.height);
            queue.triangle({sub(x, 5), y, x, sub(y, 5), x, y, border});
        }
    }
    if (!visible || elements.empty())
        return; // 0x433D0330
    const bool had_bounds = queue.is_bounds_reset();
    const auto saved_bounds = had_bounds ? queue.get_bounds() : std::array<i32, 4>{};
    if (!had_bounds)
        queue.reset_bounds(true);
    queue.set_bounds({position.x, scroll.enabled ? add(position.y, 7) : position.y, b.width,
                      scroll.enabled ? sub(b.height, 9) : b.height});
    const auto offset = scroll.enabled ? scroll.position : 0;
    const Point content{position.x, sub(position.y, offset)};
    {
        std::shared_lock lock(elements_mutex); // RtlAcquireSRWLockShared / RtlReleaseSRWLockShared
        for (auto* child : draw_order())
            if (child->visible &&
                child->geometry.bounds.y >= sub(offset, child->geometry.bounds.height))
                child->draw(queue, content, accent);
    }
    if (had_bounds)
        queue.set_bounds(saved_bounds);
    else
        queue.reset_bounds(false);
    if (!scroll.enabled)
        return;
    const auto* menu = find_menu();
    if (!menu)
        throw std::logic_error("Category scrollbar rendering requires its menu context");
    const auto scale = menu->scale;
    const auto saved_depth = queue.get_depth();
    queue.set_depth(0.25f);
    if (scroll.position > 0) {
        queue.vertical_gradient(
            {add(position.x, 2), add(position.y, 7), sub(b.width, 9), 20, 0xff171717, 0x00171717});
        draw_up_arrow(queue,
                      {sub(add(position.x, b.width), round_layout_coordinate(scale * 17.0f)),
                       add(position.y, round_layout_coordinate(scale * 9.0f))},
                      0xc8ffffff);
    }
    if (scroll.position < sub(scroll.content_height, scroll.viewport.y)) {
        queue.vertical_gradient({add(position.x, 2), add(add(position.y, b.height), -21),
                                 sub(b.width, 9), 20, 0x00171717, 0xff171717});
        draw_down_arrow(queue,
                        {sub(add(position.x, b.width), round_layout_coordinate(scale * 17.0f)),
                         add(sub(position.y, round_layout_coordinate(scale * 9.0f)), b.height)},
                        0xc8ffffff);
    }
    draw_scrollbar(queue, scroll, {add(position.x, 1), position.y}, scale);
    queue.set_depth(saved_depth);
}
}

// ============================================================================
// Subcomponent: popup_menu.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup] */
i32 mul(i32 a, std::uint32_t b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) * b);
}
}

PopupMenu::PopupMenu() {
    kind = 14;
    inline_element = true;
    geometry.horizontal_offset = 0;
}
PopupMenu::PopupMenu(Widget* owner, std::vector<std::u16string> labels,
                     WidgetIdentities& identities) // 0x4339D578
    : Widget(owner, u"", 14, true, identities), options(std::move(labels)) {
    inline_element = true;
    geometry.horizontal_offset = 0;
    geometry.default_size = {100, mul(20, std::uint32_t(options.size()))};
    geometry.bounds = {-100, 0, geometry.default_size.x, geometry.default_size.y};
}
bool PopupMenu::hit_test(Point cursor) const { // 0x4332FAE6
    if (!open)
        return false;
    if (!parent)
        throw std::logic_error("Popup hit testing requires an owning widget");
    const auto& p = parent->geometry.bounds;
    auto bounds = geometry.bounds;
    bounds.x = add(bounds.x, p.x);
    bounds.y = add(bounds.y, p.y);
    return contains(bounds, cursor);
}
void PopupMenu::set_selected(std::uint32_t index) noexcept { // 0x433C859C
    if (index != 0xffffffffU && options.size() < index)
        index = std::uint32_t(options.size());
    selected_item = index;
}
void PopupMenu::select_at(Point cursor, bool commit) { // 0x4338798F
    if (!parent)
        throw std::logic_error("Popup selection requires an owning widget");
    const auto offset = sub(sub(cursor.y, parent->geometry.bounds.y), geometry.bounds.y);
    if (offset < 0) {
        hovered_item = -1;
        return;
    }
    if (!row_height)
        throw std::domain_error("Popup row height is zero");
    const auto index = std::uint32_t(offset / row_height);
    if (index < options.size()) {
        hovered_item = std::bit_cast<i32>(index);
        if (commit) {
            if (!selected)
                throw std::logic_error("Popup selection callback is not installed");
            selected(*parent, *this, index);
        }
    }
}
bool PopupMenu::mouse(const InputMessage& message, Point cursor) { // 0x433AFC02
    if (!open)
        return false;
    if (message.message == 0x200) {
        hovered = hit_test(cursor);
        if (hovered) {
            select_at(cursor, false);
            return true;
        }
        hovered_item = -1;
    } else if (message.message == 0x201) {
        if (hit_test(cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x202 && held) {
        if (hit_test(cursor)) {
            select_at(cursor, true);
            open = false;
            held = false;
            return true;
        }
        held = false;
    }
    return false;
}
void PopupMenu::reset_input() { // 0x433CCE79
    hovered_item = -1;
    hovered = false;
    open = false;
    held = false;
}
void PopupMenu::dpi_changed(float scale, const SliderMetrics&) { // 0x433E8BC8
    row_height = round_layout_coordinate(scale * 20.0f);
    geometry.bounds.width = round_layout_coordinate(scale * 100.0f);
    geometry.bounds.height =
        add(round_layout_coordinate(scale * 0.0f), mul(row_height, std::uint32_t(options.size())));
    geometry.label_height = round_layout_coordinate(scale * 14.0f);
    if (geometry.horizontal_offset != 0)
        geometry.horizontal_offset = round_layout_coordinate(scale * 20.0f);
    geometry.bounds.x = sub(geometry.horizontal_offset, geometry.bounds.width);
    geometry.bounds.y = 0;
}
void PopupMenu::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x43330648
    if (!enabled || !open)
        return;
    if (!queue.fonts)
        throw std::logic_error("Popup rendering requires a font backend");
    const auto previous_bounds_reset = queue.is_bounds_reset();
    const auto previous_depth = queue.get_depth();
    queue.reset_bounds(false);
    queue.set_depth(0.15f);
    const auto x = add(origin.x, geometry.bounds.x), y = add(origin.y, geometry.bounds.y);
    queue.outlined_rectangle({x, y, geometry.bounds.width, geometry.bounds.height, 0xff232323U},
                             0xff0c0c0cU, 1);
    const auto text_offset = sub(row_height, font_height(*queue.fonts, 0)) / 2;
    for (std::uint32_t index = 0; index < options.size(); ++index) {
        const auto row_y = add(y, mul(row_height, index));
        const bool highlighted = index == std::uint32_t(hovered_item);
        if (highlighted)
            queue.rectangle({add(x, 1), add(row_y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), 0xff191919U});
        queue.text(add(x, 10), add(row_y, text_offset),
                   index == selected_item ? accent : 0xc8ffffffU, std::uint32_t(highlighted),
                   geometry.bounds.width, options[index]);
    }
    queue.reset_bounds(previous_bounds_reset);
    queue.set_depth(previous_depth);
}

} // namespace skeet::ui
