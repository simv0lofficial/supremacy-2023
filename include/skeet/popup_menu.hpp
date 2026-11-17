#pragma once
#include "widgets.hpp"

namespace skeet::ui {

class PopupMenu : public Widget {
public:
    PopupMenu();
    PopupMenu(Widget* owner, std::vector<std::u16string> labels, WidgetIdentities& identities);
    std::vector<std::u16string> options;
    std::int32_t row_height = 20;
    bool open = false;
    bool held = false;
    std::int32_t hovered_item = -1;
    std::uint32_t selected_item = 0xffffffffU;
    void (*selected)(Widget&, PopupMenu&, std::uint32_t) = nullptr;

    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void select_at(Point cursor, bool commit);
    void set_selected(std::uint32_t index) noexcept;
    void reset_input() override;
    void dpi_changed(float scale, const SliderMetrics&) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};

} // namespace skeet::ui
