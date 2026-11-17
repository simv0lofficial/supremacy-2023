// Menu Component: edit
#include "skeet/textbox.hpp"
#include "skeet/font_metrics.hpp"
#include "skeet/menu_tree.hpp"
#include <algorithm>
#include <bit>
#include <immintrin.h>
#include <stdexcept>


// ============================================================================
// Subcomponent: textbox.cpp
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
void acquire(std::atomic<std::uint32_t>& lock) noexcept {
    while (lock.exchange(1))
        _mm_pause();
}
}

Textbox::Textbox() {
    kind = 10;
    text_flags = 0x19;
    geometry.default_size = {158, 20};
    geometry.bounds.width = 158;
    geometry.bounds.height = 20;
}

Textbox::Textbox(Widget* owner, WidgetIdentities& identities) // 0x4336A72D
    : Widget(owner, u"", 10, false, identities) {
    text_flags = 0x19;
    geometry.default_size = {158, 20};
    geometry.bounds.width = 158;
    geometry.bounds.height = 20;
    enabled = true;
}

std::uint32_t Textbox::copy_text(std::span<char16_t, 65> destination) const { // 0x433B39DE
    if (text_length.load(std::memory_order_relaxed) == 0)
        return 0;
    acquire(text_lock);
    auto length = text_length.load(std::memory_order_relaxed);
    if (length > 0 && length <= text.size()) {
        std::copy_n(text.begin(), length, destination.begin());
        destination[length] = 0;
    } else {
        length = 0;
        destination[0] = 0;
    }
    text_lock.exchange(0);
    return length;
}

bool Textbox::mouse(const InputMessage& message, Point cursor) { // 0x4332AD0F
    return enabled && message.message == 0x201 && hit_test(cursor);
}

bool Textbox::keyboard(const InputMessage& message) { // 0x43344B4D
    if (message.message == 0x100) {
        if (message.parameter == 8) {
            if (text_length.load(std::memory_order_relaxed) != 0) {
                const auto* menu = find_menu();
                if (!menu)
                    throw std::logic_error("Textbox backspace requires its menu context");
                if (menu->button_flags & 8)
                    text_length.exchange(0);
                else {
                    auto length = text_length.load(std::memory_order_relaxed);
                    while (length != 0 &&
                           !text_length.compare_exchange_strong(length, length - 1)) {
                    }
                }
            }
        } else if (message.parameter == 0x1b) {
            if (auto* tab = owning_tab(this))
                tab->focus_widget(nullptr);
        }
    } else {
        if (message.message != 0x102)
            return false;
        if (message.parameter != 8 && message.parameter != 0x60 &&
            text_length.load(std::memory_order_relaxed) < 63) {
            acquire(text_lock);
            const auto length = text_length.load(std::memory_order_relaxed);
            if (length < 63) {
                text[length] = char16_t(message.parameter);
                text_length.exchange(length + 1);
            }
            text_lock.exchange(0);
        }
    }
    return true;
}

void Textbox::resize(const SliderMetrics& metrics) { // 0x4331E326
    if (!parent)
        throw std::logic_error("Textbox resizing requires an owning category");
    auto width = sub(parent->geometry.bounds.width, metrics.category_padding);
    if (width > metrics.preferred_maximum_width)
        width = metrics.preferred_maximum_width;
    else if (width < metrics.minimum_width)
        width = metrics.minimum_width;
    geometry.bounds.width = width;
}

void Textbox::draw(RenderQueue& queue, Point origin, std::uint32_t accent) const { // 0x43364B5C
    const auto& bounds = geometry.bounds;
    const auto x = add(origin.x, bounds.x), y = add(origin.y, bounds.y);
    queue.outlined_rectangle(
        {x, y, bounds.width, bounds.height, enabled ? 0xff232323U : 0x50232323U}, 0xff0c0c0cU, 1);
    if (!enabled)
        return;
    queue.outlined_rectangle(
        {add(x, 1), add(y, 1), sub(bounds.width, 2), sub(bounds.height, 2), 0xff191919U},
        0xff323232U, 1);
    queue.rectangle_outline(
        {add(x, 2), add(y, 2), sub(bounds.width, 4), sub(bounds.height, 4), 0xff101010U}, 1);
    std::array<char16_t, 65> display{};
    const auto length = copy_text(display);
    display[length] = u'_';
    if (!queue.fonts)
        throw std::logic_error("Textbox rendering requires its font backend");
    const auto height = font_height(*queue.fonts, 0);
    const auto* tab = owning_tab(const_cast<Textbox*>(this));
    const auto focused = tab && const_cast<Tab*>(tab)->focused_widget() == this;
    queue.text(add(x, 5), add(y, sub(bounds.height, height) / 2), focused ? accent : 0xc8ffffffU, 0,
               sub(bounds.width, 10), {display.data(), length + 1});
}
} // namespace skeet::ui
