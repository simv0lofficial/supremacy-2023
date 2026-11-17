#pragma once
#include "core.hpp"

namespace skeet::ui {

struct ElementGeometry {
    Rectangle bounds{};
    Point default_size{};
    std::int32_t horizontal_offset = 0;
    std::int32_t label_height = 0;
};

std::int32_t round_layout_coordinate(float value) noexcept;
Point scale_size(Point size, float scale) noexcept;
void resize_element(ElementGeometry& element, float scale) noexcept;
void move_scroll_position(std::int32_t& position, std::int32_t viewport_height,
                          std::int32_t content_height, std::int32_t delta) noexcept;

struct CategoryGeometry {
    ElementGeometry element;
    std::int32_t fixed_height = 0;
    std::int32_t margin = 10;
    std::int8_t column = 0, column_span = 24, row = 0, row_span = 24;
};
struct MenuGeometry {
    Point size{};
    Point start_padding{}, end_padding{};
};
struct TabGeometry {
    std::int32_t width = 75;
    std::int32_t padding = 10;
    std::span<CategoryGeometry*> categories;
};
void layout_category_grid(const MenuGeometry& menu, const TabGeometry& tab);

struct LayoutElement {
    ElementGeometry geometry;
    bool visible = true;
    bool inline_element = false;
    std::uint8_t kind = 0;
    void* context = nullptr;
    void (*layout)(LayoutElement&) = nullptr;
    void (*reset)(LayoutElement&) = nullptr;
};
struct Scrollbar {
    bool enabled = false;
    Point viewport{};
    std::int32_t content_height = 0;
    std::int32_t position = 0;
    bool dragging = false;
    std::int32_t grab_offset = 0;
};
Rectangle scrollbar_thumb(const Scrollbar& scroll, std::int32_t width, float scale);
bool scrollbar_mouse(Scrollbar& scroll, std::uint32_t message, Point cursor, float scale);
struct CategoryLayout {
    Rectangle bounds{};
    std::int32_t padding = 20;
    std::span<LayoutElement*> elements;
    Scrollbar scroll;
};
void reflow_category(CategoryLayout& category);

struct ListboxLayout {
    ElementGeometry element;
    std::int32_t row_height = 18;
    bool search_visible = false;
    std::int32_t visible_rows = 0;
    std::int32_t item_count = 0;
    Scrollbar scroll;
};
void update_listbox_scroll(ListboxLayout& listbox) noexcept;
void reflow_listbox(ListboxLayout& listbox, float scale);

} // namespace skeet::ui
