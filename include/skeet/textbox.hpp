#pragma once
#include "widgets.hpp"

namespace skeet::ui {
class Textbox : public Widget {
public:
    Textbox();
    Textbox(Widget* owner, WidgetIdentities& identities);

    mutable std::atomic<std::uint32_t> text_lock{0};
    std::atomic<std::uint32_t> text_length{0};
    std::array<char16_t, 64> text{};

    // The native reader permits 64 externally supplied characters; keyboard
    // input stops at 63. A zero length leaves the destination untouched.
    std::uint32_t copy_text(std::span<char16_t, 65> destination) const;
    bool mouse(const InputMessage& message, Point cursor) override;
    bool keyboard(const InputMessage& message) override;
    void resize(const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};
} // namespace skeet::ui
