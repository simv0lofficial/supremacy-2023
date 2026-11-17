#pragma once
#include "widgets.hpp"

namespace skeet::ui {
struct MultiSelectOption {
    std::string name;
    std::uint32_t mask = 0;
};

class MultiSelect : public Widget {
public:
    MultiSelect();
    MultiSelect(Widget* owner, std::u16string_view label, std::uint32_t* setting,
                WidgetIdentities& identities, bool require_one = true);
    MultiSelect(Widget* owner, std::u16string_view label, std::int32_t* setting,
                WidgetIdentities& identities, bool require_one = true);
    std::uint32_t* value = nullptr;
    std::int32_t row_height = 20, interaction_offset = 15;
    bool open = false, held = false, require_at_least_one = true;
    std::int32_t hovered_item = -1;
    std::string display;
    std::vector<MultiSelectOption> options;
    Scrollbar scroll;
    float scale = 1.0f;

    void add_option(std::string_view label, std::uint32_t mask);
    void set_value(std::uint32_t requested);
    std::uint32_t selected_count() const;
    void refresh_display();
    void value_changed() override;
    void config_loaded() override;
    void reset_input() override;
    std::int32_t input_priority() const override;
    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    bool interact_options(Point cursor, bool commit);
    void update_scroll() noexcept;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float factor, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};
}
