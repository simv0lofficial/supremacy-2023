#pragma once
#include "widgets.hpp"

namespace skeet::ui {
struct ListboxOption {
    std::int32_t value = 0;
    std::u16string name;
};

class Listbox : public Widget {
public:
    Listbox();
    Listbox(Widget* owner, std::u16string_view label, Point size, std::int32_t* setting,
            bool show_search, WidgetIdentities& identities);
    std::int32_t* value = nullptr;
    std::int32_t row_height = 18, visible_rows = 0, selected_item = 0;
    std::int32_t original_height = 0;
    bool search_visible = false, search_active = false;
    float scale = 1.0f;
    Scrollbar scroll;
    std::vector<ListboxOption> options;
    std::u16string search;
    std::vector<std::uint16_t> filtered_indices;

    void set_selection(std::uint32_t index) noexcept;
    void set_options(std::vector<ListboxOption>* replacement);
    void filter_options();
    void update_scroll() noexcept;
    std::int32_t option_at(Point relative_cursor) const;
    bool mouse(const InputMessage& message, Point cursor) override;
    bool keyboard(const InputMessage& message) override;
    void reset_input() override;
    void config_loaded() override;
    void value_changed() override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float factor, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};
} // namespace skeet::ui
