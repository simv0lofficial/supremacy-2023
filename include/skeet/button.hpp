#pragma once
#include "widgets.hpp"

namespace skeet::ui {

class Button : public Widget {
public:
    Button();
    Button(Widget* owner, std::u16string_view label, Point size, WidgetCallback action,
           WidgetIdentities& identities);

    bool held = false;

    bool mouse(const InputMessage& message, Point cursor) override;
    void reset_input() override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent = 0) const override;
};

} // namespace skeet::ui
