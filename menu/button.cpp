// Menu Component: button
#include "skeet/button.hpp"
#include <bit>
#include <stdexcept>


// ============================================================================
// Subcomponent: button.cpp
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
}

Button::Button() {
    kind = 2;
    text_flags = 0x18;
}

Button::Button(Widget* owner, std::u16string_view label, Point size, WidgetCallback action,
               WidgetIdentities& identities) // 0x433665D5
    : Widget(owner, label, 2, false, identities) {
    geometry.default_size = size;
    geometry.bounds.width = size.x;
    geometry.bounds.height = size.y;
    text_flags = 0x18;
    callbacks.push_back(action);
    enabled = true;
}

bool Button::mouse(const InputMessage& message, Point cursor) { // 0x433ADA41
    if (message.message == 0x200) {
        hovered = hit_test(cursor);
        if (!hovered)
            held = false;
    } else if (message.message == 0x201) {
        if (hit_test(cursor)) {
            held = true;
            return true;
        }
    } else if (message.message == 0x202 && held) {
        held = false;
        if (hit_test(cursor))
            notify_callbacks();
        return true;
    }
    return false;
}

void Button::reset_input() { // 0x433D6CB9
    hovered = false;
    held = false;
}

void Button::resize(const SliderMetrics& metrics) { // 0x433D65F5
    if (!parent)
        throw std::logic_error("Button resizing requires an owning category");
    auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    if (width < metrics.minimum_width)
        width = metrics.minimum_width;
    else if (width > metrics.preferred_maximum_width)
        width = metrics.preferred_maximum_width;
    geometry.bounds.width = width;
}

void Button::dpi_changed(float scale, const SliderMetrics&) { // 0x4336EDA0
    resize_element(geometry, scale);
}

void Button::draw(RenderQueue& queue, Point origin, std::uint32_t) const { // 0x43359006
    const auto& bounds = geometry.bounds;
    const auto x = add(origin.x, bounds.x);
    const auto y = add(origin.y, bounds.y);
    const auto height = sub(bounds.height, 3);
    queue.rectangle_outline({x, y, bounds.width, height, 0xff0c0c0cU}, 1);
    queue.rectangle_outline(
        {add(x, 1), add(y, 1), sub(bounds.width, 2), sub(height, 2), 0xff323232U}, 1);
    const auto top = held ? 0xff1e1e1eU : hovered ? 0xff272727U : 0xff222222U;
    const auto bottom = held ? 0xff141414U : hovered ? 0xff232323U : 0xff1e1e1eU;
    queue.vertical_gradient(
        {add(x, 2), add(y, 2), sub(bounds.width, 4), sub(height, 4), top, bottom});
    const auto label = std::u16string_view(name).substr(0, 256);
    if (!label.empty()) {
        queue.text(add(x, bounds.width / 2), add(y, height / 2),
                   enabled ? 0xc8ffffffU : 0xff3c3c3cU, text_flags, sub(bounds.width, 6), label);
    }
}

} // namespace skeet::ui
