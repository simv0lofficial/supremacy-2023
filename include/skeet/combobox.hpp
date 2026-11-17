#pragma once
#include "widgets.hpp"

namespace skeet::ui {
struct ComboBoxOption {
    std::u16string name;
    std::uint32_t value = 0;
};

class ComboBox : public Widget {
public:
    ComboBox();
    ComboBox(Widget* owner, std::u16string_view label, std::uint32_t* setting, bool same_line,
             WidgetIdentities& identities);
    ComboBox(Widget* owner, std::u16string_view label, std::int32_t* setting, bool same_line,
             WidgetIdentities& identities);
    std::uint32_t* value = nullptr;
    std::int32_t default_row_height = 20, row_height = 20;
    std::int32_t default_interaction_offset = 15, interaction_offset = 15;
    bool open = false, held = false;
    std::int32_t hovered_item = -1;
    std::uint32_t selected_item = 0;
    std::vector<ComboBoxOption> options;
    Scrollbar scroll;
    float scale = 1.0f;

    void set_options(std::vector<ComboBoxOption> replacement);
    void set_value(std::uint32_t requested);
    void value_changed() override;
    void config_loaded() override;
    void reset_input() override;
    std::int32_t input_priority() const override;
    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void interact_options(const InputMessage& message, Point cursor);
    void update_scroll() noexcept;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float factor, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    void draw_box(RenderQueue& queue, Point position) const;
    void draw_option(RenderQueue& queue, Point position, std::int32_t height, std::uint32_t index,
                     std::uint32_t text_color, std::uint32_t flags) const;
};
}
