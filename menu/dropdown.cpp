// Menu Component: dropdown
#include "skeet/combobox.hpp"
#include "skeet/font_metrics.hpp"
#include <algorithm>
#include <bit>
#include <limits>
#include <stdexcept>
#include "skeet/listbox.hpp"
#include "skeet/menu_tree.hpp"
#include <mutex>


// ============================================================================
// Subcomponent: combobox.cpp
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
        throw std::overflow_error("Invalid combobox integer division");
    return a / b;
}
}

ComboBox::ComboBox() {
    kind = 4;
}
ComboBox::ComboBox(Widget* owner, std::u16string_view label, std::uint32_t* setting, bool same_line,
                   WidgetIdentities& identities) // 0x43399927
    : Widget(owner, label, 4, setting != nullptr, identities), value(setting) {
    inline_element = same_line;
    geometry.default_size = {158, !name.empty() && !same_line ? 35 : 20};
    geometry.bounds.width = geometry.default_size.x;
    geometry.bounds.height = geometry.default_size.y;
    update_scroll();
}
void ComboBox::set_options(std::vector<ComboBoxOption> replacement) { // 0x433EE01C
    options = std::move(replacement);
    const auto current = value ? *value : 0;
    for (std::size_t i = 0; i < options.size(); ++i)
        if (options[i].value == current)
            selected_item = static_cast<std::uint32_t>(i);
    update_scroll();
}
ComboBox::ComboBox(Widget* owner, std::u16string_view label, std::int32_t* setting, bool same_line,
                   WidgetIdentities& identities)
    // C++ permits access through the corresponding signed/unsigned type.
    // Preserve the setting's original 32-bit storage rather than copying it.
    : ComboBox(owner, label, reinterpret_cast<std::uint32_t*>(setting), same_line, identities) {}
void ComboBox::set_value(std::uint32_t requested) { // 0x433F49B3
    if ((value ? *value : 0) != requested) {
        if (value)
            *value = requested;
        value_changed();
    }
}
void ComboBox::value_changed() { // 0x433ADEE6
    const auto current = value ? *value : 0;
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (options[i].value == current) {
            selected_item = static_cast<std::uint32_t>(i);
            break;
        }
    }
    notify_callbacks();
}
void ComboBox::config_loaded() { // 0x43383770
    if (!value)
        return;
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (options[i].value == *value) {
            selected_item = static_cast<std::uint32_t>(i);
            return;
        }
    }
    if (!options.empty())
        set_value(options.front().value);
}
void ComboBox::reset_input() { // 0x433FA887
    hovered_item = -1;
    hovered = false;
    open = held = false;
    scroll.dragging = false;
}
i32 ComboBox::input_priority() const {
    return open ? 2 : 0;
} // 0x4338A0B0
bool ComboBox::hit_test(Point cursor) const { // 0x43388257
    auto bounds = geometry.bounds;
    if (open)
        bounds.height = add(
            bounds.height,
            add(1, mul(static_cast<i32>(std::min<std::size_t>(8, options.size())), row_height)));
    if (!name.empty() && !inline_element) {
        bounds.height = sub(bounds.height, interaction_offset);
        bounds.y = add(bounds.y, interaction_offset);
    }
    return contains(bounds, cursor);
}
void ComboBox::interact_options(const InputMessage& message, Point cursor) { // 0x43325388
    const auto y = sub(sub(cursor.y, geometry.bounds.height), geometry.bounds.y);
    if (y < 0)
        hovered_item = -1;
    else {
        const auto offset =
            scroll.enabled ? mul(divide(scroll.position, row_height), row_height) : 0;
        const auto index = static_cast<std::uint32_t>(divide(add(offset, y), row_height));
        if (index < options.size()) {
            hovered_item = divide(y, row_height);
            if (held && (value ? *value : 0) != options[index].value)
                set_value(options[index].value);
        }
    }
    if (message.message == 0x202)
        open = held = false;
}
bool ComboBox::mouse(const InputMessage& message, Point cursor) { // 0x4340C49A
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
        const auto wheel =
            std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(message.parameter >> 16));
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height,
                             mul(row_height, wheel > 0 ? -4 : 4));
        return true;
    }
    return false;
}
void ComboBox::update_scroll() noexcept { // 0x433B09E2
    if (options.size() < 9) {
        scroll.enabled = false;
        return;
    }
    scroll.enabled = true;
    scroll.viewport = {geometry.bounds.width, mul(row_height, 8)};
    scroll.content_height = mul(static_cast<i32>(options.size()), row_height);
    move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height, 0);
}
void ComboBox::resize(const SliderMetrics& metrics) { // 0x4332BB67
    const auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    geometry.bounds.width = width < metrics.minimum_width
                                ? metrics.minimum_width
                                : std::min(width, metrics.preferred_maximum_width);
    update_scroll();
}
void ComboBox::dpi_changed(float factor, const SliderMetrics&) { // 0x43367728
    scale = factor;
    resize_element(geometry, factor);
    row_height = round_layout_coordinate(float(default_row_height) * factor);
    interaction_offset = round_layout_coordinate(float(default_interaction_offset) * factor);
    update_scroll();
}
void ComboBox::draw_box(RenderQueue& queue, Point position) const { // 0x43327D7A
    const bool highlight = hovered || open;
    const auto first = highlight ? 0xff292929U : enabled ? 0xff1f1f1fU : 0x501f1f1fU;
    const auto second = highlight ? 0xff2e2e2eU : enabled ? 0xff242424U : 0x50242424U;
    queue.vertical_gradient({add(position.x, 1), add(position.y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), first, second});
    queue.rectangle_outline({position.x, position.y, geometry.bounds.width, row_height, 0xff0c0c0c},
                            1);
}
void ComboBox::draw_option(RenderQueue& queue, Point position, i32 height, std::uint32_t index,
                           std::uint32_t text_color, std::uint32_t flags) const { // 0x433B1508
    if (index >= options.size() || options[index].name.empty())
        return;
    if (!queue.fonts)
        throw std::logic_error("Combobox text requires its native font backend");
    const auto text_height = font_height(*queue.fonts, flags & 1);
    queue.text(position.x, add(position.y, divide(sub(height, text_height), 2)), text_color, flags,
               sub(geometry.bounds.width, round_layout_coordinate(scale * 20.0f)),
               options[index].name);
}
void ComboBox::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x433B8D0F
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    draw_label(queue, position, sub(sub(parent->geometry.bounds.width, geometry.bounds.x), 8));
    const auto text_color = enabled ? 0x88ffffffU : 0x32ffffffU;
    draw_box(queue, position);
    const auto height = !name.empty() && !inline_element
                            ? sub(geometry.bounds.height, interaction_offset)
                            : geometry.bounds.height;
    draw_option(queue, {add(position.x, round_layout_coordinate(scale * 7.0f)), position.y}, height,
                selected_item, text_color, 0);
    const auto arrow_x =
        add(sub(position.x, round_layout_coordinate(scale * 12.0f)), geometry.bounds.width);
    const auto arrow_y = add(position.y, divide(sub(height, 4), 2));
    queue.rectangle({arrow_x, sub(arrow_y, 1), 5, 1, 0xff000000});
    queue.rectangle({arrow_x, arrow_y, 5, 1, text_color});
    queue.rectangle({add(arrow_x, 1), add(arrow_y, 1), 3, 1, text_color});
    queue.rectangle({add(arrow_x, 2), add(arrow_y, 2), 1, 1, text_color});
    if (!enabled || !open)
        return;
    const auto bounds_reset = queue.is_bounds_reset();
    const auto depth = queue.get_depth();
    queue.reset_bounds(false);
    queue.set_depth(0.15f);
    const Point popup{add(origin.x, geometry.bounds.x),
                      add(add(origin.y, geometry.bounds.y), add(geometry.bounds.height, 1))};
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
        const auto highlight = row == static_cast<std::uint32_t>(hovered_item);
        if (highlight)
            queue.rectangle({add(popup.x, 1), add(row_y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), 0xff191919});
        draw_option(queue, {add(popup.x, round_layout_coordinate(scale * 7.0f)), row_y}, row_height,
                    index, index == selected_item ? accent : 0xc8ffffffU,
                    highlight || index == selected_item ? 1 : 0);
    }
    if (scroll.enabled) { // 0x433E7AFE
        const auto width = round_layout_coordinate(scale * 7.0f);
        queue.rectangle({sub(add(popup.x, scroll.viewport.x), add(width, 1)), add(popup.y, 1),
                         width, sub(scroll.viewport.y, 2), 0xff282828});
        const auto thumb = scrollbar_thumb(scroll, 7, scale);
        queue.rectangle(
            {add(popup.x, thumb.x), add(popup.y, thumb.y), thumb.width, thumb.height, 0xff414141});
    }
    queue.reset_bounds(bounds_reset);
    queue.set_depth(depth);
}
}

// ============================================================================
// Subcomponent: listbox.cpp
// ============================================================================

namespace skeet::ui {
namespace {
/* [dedup arithmetic helpers] */
char16_t lower_ascii(char16_t c) noexcept {
    return c >= u'A' && c <= u'Z' ? c + 32 : c;
}
bool contains_search(std::u16string_view name, std::u16string_view query) {
    name = name.substr(0, name.find(u'\0'));
    query = query.substr(0, query.find(u'\0'));
    if (query.empty())
        return false; // 0x433D27C7 returns null for an empty needle.
    return std::search(name.begin(), name.end(), query.begin(), query.end(),
                       [](char16_t a, char16_t b) { return lower_ascii(a) == lower_ascii(b); }) !=
           name.end();
}
void arrow(RenderQueue& q, Point p, bool down) {
    q.rectangle({p.x, add(p.y, down ? -1 : 1), 5, 1, 0xff000000U});
    q.rectangle({p.x, p.y, 5, 1, 0xc8ffffffU});
    q.rectangle({add(p.x, 1), add(p.y, down ? 1 : -1), 3, 1, 0xc8ffffffU});
    q.rectangle({add(p.x, 2), add(p.y, down ? 2 : -2), 1, 1, 0xc8ffffffU});
}
}
Listbox::Listbox() {
    kind = 6;
}
Listbox::Listbox(Widget* owner, std::u16string_view label, Point size, i32* setting,
                 bool show_search, WidgetIdentities& identities) // 0x433EDA39
    : Widget(owner, label, 6, setting != nullptr, identities), value(setting),
      original_height(size.y), search_visible(show_search) {
    geometry.default_size = size;
    geometry.bounds.width = size.x;
    geometry.bounds.height = size.y;
    visible_rows = sub(divide(size.y, row_height), search_visible ? 1 : 0);
    if (size.x != 158)
        geometry.horizontal_offset = 0;
}
void Listbox::set_selection(std::uint32_t index) noexcept { // 0x433B0844
    if (index < options.size())
        selected_item = std::bit_cast<i32>(index);
}
void Listbox::set_options(std::vector<ListboxOption>* replacement) { // 0x433C9FBF
    auto* category = owning_category(this);
    if (!category)
        return;
    const auto selection = replacement ? std::uint32_t(selected_item) : 0U;
    const auto position = replacement && scroll.enabled ? scroll.position : 0;
    std::unique_lock lock(category->elements_mutex);
    set_selection(0);
    options.clear();
    filtered_indices.clear();
    if (replacement && replacement != &options)
        options.swap(*replacement);
    filter_options();
    set_selection(
        options.empty() ? 0 : std::min(selection, static_cast<std::uint32_t>(options.size() - 1)));
    scroll.position = position;
    visible_rows = sub(divide(geometry.bounds.height, row_height), search_visible ? 1 : 0);
    update_scroll();
}
void Listbox::filter_options() { // 0x433F27E3
    scroll.position = 0;
    if (search.empty()) {
        if (filtered_indices.size() != options.size()) {
            filtered_indices.resize(options.size());
            for (std::size_t i = 0; i < options.size(); ++i)
                filtered_indices[i] = static_cast<std::uint16_t>(i);
        }
    } else {
        filtered_indices.clear();
        filtered_indices.reserve(options.size());
        for (std::size_t i = 0; i < options.size(); ++i)
            if (contains_search(options[i].name, search))
                filtered_indices.push_back(static_cast<std::uint16_t>(i));
    }
    update_scroll();
}
void Listbox::update_scroll() noexcept { // 0x4333FF11
    ListboxLayout layout{geometry,
                         row_height,
                         search_visible,
                         visible_rows,
                         static_cast<i32>(filtered_indices.size()),
                         scroll};
    update_listbox_scroll(layout);
    scroll = layout.scroll;
}
i32 Listbox::option_at(Point cursor) const { // 0x4336B2E0
    if (filtered_indices.empty() || cursor.y < 0)
        return -1;
    const auto offset = scroll.enabled ? mul(divide(scroll.position, row_height), row_height) : 0;
    const auto index = divide(add(offset, cursor.y), row_height);
    return index >= 0 && std::uint32_t(index) < filtered_indices.size() ? filtered_indices[index]
                                                                        : -1;
}
bool Listbox::mouse(const InputMessage& message, Point cursor) { // 0x4336C227
    if (options.empty())
        return false;
    Point relative{sub(cursor.x, geometry.bounds.x), sub(cursor.y, geometry.bounds.y)};
    if (search_visible)
        relative.y = sub(relative.y, row_height);
    if (scroll.enabled && scrollbar_mouse(scroll, message.message, relative, scale))
        return true;
    if (message.message == 0x201 && hit_test(cursor)) {
        if (relative.y < 0)
            search_active = true;
        else {
            const auto index = option_at(relative);
            if (index == -1)
                return false;
            set_selection(std::uint32_t(index));
            if (value)
                *value = options[selected_item].value;
            notify_callbacks();
        }
        return true;
    }
    if (message.message == 0x202)
        scroll.dragging = false;
    else if (message.message == 0x20a && scroll.enabled && hit_test(cursor)) {
        const auto wheel =
            std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(message.parameter >> 16));
        move_scroll_position(scroll.position, scroll.viewport.y, scroll.content_height,
                             mul(row_height, wheel > 0 ? -4 : 4));
        return true;
    }
    return false;
}
bool Listbox::keyboard(const InputMessage& message) { // 0x433F2387
    if (options.empty())
        return false;
    const auto key = message.parameter;
    if (message.message == 0x100) {
        if (key == 0x26) {
            if (selected_item > 0) {
                set_selection(std::uint32_t(sub(selected_item, 1)));
                if (value)
                    *value = options[selected_item].value;
                notify_callbacks();
                const auto position = mul(selected_item, row_height);
                if (position < mul(divide(scroll.position, row_height), row_height)) {
                    scroll.position = position;
                    update_scroll();
                }
            }
            return true;
        }
        if (key == 0x28) {
            if (add(selected_item, 1) < static_cast<i32>(options.size())) {
                set_selection(std::uint32_t(add(selected_item, 1)));
                if (value)
                    *value = options[selected_item].value;
                notify_callbacks();
                if (mul(selected_item, row_height) >=
                    mul(add(divide(scroll.position, row_height), visible_rows), row_height)) {
                    scroll.position = mul(add(sub(selected_item, visible_rows), 1), row_height);
                    update_scroll();
                }
            }
            return true;
        }
        if (key == 0xc0) {
            reset_input();
            return false;
        }
        if (scroll.enabled && static_cast<i32>(options.size()) >= visible_rows &&
            (key == 0x21 || key == 0x22)) {
            move_scroll_position(
                scroll.position, scroll.viewport.y, scroll.content_height,
                mul(key == 0x21 ? sub(1, visible_rows) : sub(visible_rows, 1), row_height));
            return true;
        }
        if (!search_active || !search_visible)
            return false;
        if (key == 8) {
            if (search.empty())
                return false;
            search.pop_back();
            filter_options();
            return true;
        }
        return key >= 0x20 && key <= 0x7e;
    }
    if (message.message != 0x102 || !search_active || !search_visible || key < 0x20 || key > 0x7e ||
        search.size() >= 16)
        return false;
    search.push_back(static_cast<char16_t>(key));
    filter_options();
    return true;
}
void Listbox::config_loaded() { // 0x433293EA
    if (!value)
        return;
    for (std::size_t i = 0; i < options.size(); ++i)
        if (options[i].value == *value) {
            set_selection(static_cast<std::uint32_t>(i));
            return;
        }
}
void Listbox::value_changed() {
    config_loaded();
    notify_callbacks();
} // 0x433EE5AD
void Listbox::reset_input() {
    hovered = search_active = scroll.dragging = false;
} // 0x433BA433
void Listbox::resize(const SliderMetrics& metrics) { // 0x4333C9A7
    if (!parent)
        throw std::logic_error("Listbox resizing requires an owning category");
    const auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    geometry.bounds.width = width < metrics.minimum_width
                                ? metrics.minimum_width
                                : std::min(width, metrics.preferred_maximum_width);
    visible_rows = sub(divide(geometry.bounds.height, row_height), search_visible ? 1 : 0);
    update_scroll();
}
void Listbox::dpi_changed(float factor, const SliderMetrics&) { // 0x4339E4DF
    scale = factor;
    ListboxLayout layout{geometry,
                         row_height,
                         search_visible,
                         visible_rows,
                         static_cast<i32>(filtered_indices.size()),
                         scroll};
    reflow_listbox(layout, factor);
    geometry = layout.element;
    row_height = layout.row_height;
    visible_rows = layout.visible_rows;
    scroll = layout.scroll;
}
void Listbox::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x434179DE
    if (!queue.fonts)
        throw std::logic_error("Listbox rendering requires its font backend");
    const auto text_offset = divide(sub(row_height, font_height(*queue.fonts, 0)), 2);
    Point position{add(origin.x, geometry.bounds.x), add(origin.y, geometry.bounds.y)};
    const auto alpha = enabled ? 0xff000000U : 0x50000000U;
    queue.outlined_rectangle(
        {position.x, position.y, geometry.bounds.width, geometry.bounds.height, alpha | 0x232323},
        0xff0c0c0c, 1);
    auto text_color = [&](std::uint32_t color) {
        return enabled ? color : (color & 0xffffffU) | 0x50000000U;
    };
    if (search_visible) {
        queue.outlined_rectangle({add(position.x, 1), add(position.y, 1),
                                  sub(geometry.bounds.width, 2), row_height, alpha | 0x191919},
                                 alpha | 0x323232, 1);
        queue.rectangle_outline({add(position.x, 2), add(position.y, 2),
                                 sub(geometry.bounds.width, 4), sub(row_height, 2),
                                 alpha | 0x101010},
                                1);
        auto display = search.substr(0, search.find(u'\0'));
        display.push_back(u'_');
        queue.text(add(position.x, 5), add(position.y, text_offset),
                   text_color(search_active ? accent : 0xc8ffffffU), 0, geometry.bounds.width,
                   display);
        position.y = add(position.y, row_height);
    }
    if (options.empty())
        return;
    const auto rows =
        std::min(static_cast<std::uint32_t>(options.size()),
                 static_cast<std::uint32_t>(
                     sub(divide(geometry.bounds.height, row_height), search_visible ? 1 : 0)));
    const auto offset = scroll.enabled ? divide(scroll.position, row_height) : 0;
    auto y = position.y;
    for (std::uint32_t row = 0; row < rows; ++row) {
        const auto filtered = row + std::uint32_t(offset);
        if (filtered >= filtered_indices.size())
            break;
        const auto index = filtered_indices[filtered];
        const auto& option = options.at(index);
        const auto selected = std::uint32_t(selected_item) == index;
        if (selected)
            queue.rectangle({add(position.x, 1), add(y, 1), sub(geometry.bounds.width, 2),
                             sub(row_height, 2), 0xff1a1a1a});
        const auto color =
            option.value < 0 || (value && option.value == *value) ? accent : 0xc8ffffffU;
        queue.text(add(position.x, 10), add(y, text_offset), text_color(color), selected ? 1 : 0,
                   sub(geometry.bounds.width, 20), option.name);
        y = add(y, row_height);
    }
    if (!scroll.enabled)
        return;
    const auto pixels = mul(divide(scroll.position, row_height), row_height);
    if (pixels > 0)
        arrow(queue, {sub(add(position.x, geometry.bounds.width), 20), add(position.y, 8)}, false);
    if (add(visible_rows, pixels) < static_cast<i32>(filtered_indices.size())) {
        auto arrow_y = add(sub(position.y, 9), geometry.bounds.height);
        if (search_visible)
            arrow_y = sub(arrow_y, row_height);
        arrow(queue, {sub(add(position.x, geometry.bounds.width), 20), arrow_y}, true);
    }
    const auto width = round_layout_coordinate(scale * 7.0f);
    queue.rectangle({sub(add(position.x, scroll.viewport.x), add(width, 1)), add(position.y, 1),
                     width, sub(scroll.viewport.y, 2), alpha | 0x282828});
    const auto thumb = scrollbar_thumb(scroll, 7, scale);
    queue.rectangle({add(position.x, thumb.x), add(position.y, thumb.y), thumb.width, thumb.height,
                     alpha | 0x414141});
}
} // namespace skeet::ui
