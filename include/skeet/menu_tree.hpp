#pragma once
#include "widgets.hpp"
#include <shared_mutex>

namespace skeet::ui {
struct GridPosition {
    std::int8_t column = 0, column_span = 24, row = 0, row_span = 24;
};
class Category : public Widget {
public:
    Category() {
        kind = 7;
    }
    Category(Widget* owner, std::u16string_view label, GridPosition position,
             std::size_t reserve_elements, bool allow_resize, WidgetIdentities& identities);
    std::vector<Widget*> elements;
    mutable std::shared_mutex elements_mutex;
    std::uint32_t background = 0xff171717;
    std::uint32_t title_color = 0xc8ffffff, inner_border = 0xff282828, outer_border = 0xff0c0c0c;
    bool show_title = true;
    std::int32_t drag_state = 0;
    Point drag_origin{};
    std::int8_t column = 0, column_span = 24, row = 0, row_span = 24;
    std::int32_t default_fixed_height = 0, fixed_height = 0;
    std::int32_t padding = 20, margin = 10;
    bool resizable = false, movable = false;
    Scrollbar scroll;
    void reset_children();
    void reset_input() override;
    bool keyboard(const InputMessage& message) override;
    void config_loaded() override;
    bool mouse(const InputMessage& message, Point cursor) override;
    bool move(Point cursor);
    bool change_size(Point cursor);
    std::vector<Widget*> input_order() const;
    std::vector<Widget*> draw_order() const;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    std::int32_t input_priority() const override {
        return -1;
    } // 0x43336955
    void reflow();
    void visibility_changed() override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
};

struct FocusPath {
    std::uint32_t category = UINT32_MAX;
    std::uint32_t element = UINT32_MAX;
    std::uint32_t identity = UINT32_MAX;
};
class Tab : public Widget {
public:
    Tab() {
        kind = 1;
    }
    Tab(Menu& menu, MenuInput& input, MenuGeometry& layout, std::u16string_view label,
        Point& next_position, WidgetIdentities& identities);
    std::vector<Category*> categories;
    FocusPath focus;
    Menu* navigation = nullptr;
    MenuGeometry local_menu_geometry{};
    MenuGeometry& menu_geometry = local_menu_geometry;
    std::int32_t padding = 10;
    std::uint32_t icon = UINT32_MAX;
    Rectangle icon_source{};
    Widget* focused_widget() noexcept;
    FocusPath path_to(const Widget* target) const noexcept;
    void focus_widget(Widget* target);
    void reset_input() override;
    bool keyboard(const InputMessage& message) override;
    void config_loaded() override;
    bool mouse(const InputMessage& message, Point cursor) override;
    virtual void layout_categories();
    virtual void activate() {} // 0x433CCE8E in the base tab vtable
    void reflow_categories();
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};
Category* owning_category(Widget* widget) noexcept;
Tab* owning_tab(Widget* widget) noexcept;
} // namespace skeet::ui
