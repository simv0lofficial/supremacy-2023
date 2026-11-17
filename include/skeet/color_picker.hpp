#pragma once
#include "popup_menu.hpp"

namespace skeet::ui {

struct HsvColor {
    float hue = 0, saturation = 0, value = 0;
};
HsvColor rgb_to_hsv(std::uint32_t color) noexcept;
std::uint32_t hsv_to_rgb(HsvColor color) noexcept;
std::array<char, 9> format_color_clipboard(std::uint32_t color) noexcept;
bool parse_color_clipboard(const char* text, std::uint32_t& color) noexcept;

struct ColorClipboard {
    void* context = nullptr;
    void (*write)(void*, std::uint32_t) = nullptr;
    bool (*read)(void*, std::uint32_t&) = nullptr;
};
ColorClipboard& system_color_clipboard() noexcept;

enum class PickerStatus : std::int32_t {
    idle,
    clicked,
    tint,
    hue,
    alpha
};

class ColorPicker : public Widget {
public:
    ColorPicker();
    ColorPicker(Widget* owner, std::uint32_t* setting, WidgetIdentities& identities);
    std::uint32_t* value = nullptr;
    Rectangle picker_bounds{0, 11, 180, 175};
    Rectangle tint_bounds{5, 16, 150, 150};
    Rectangle hue_bounds{160, 16, 15, 150};
    Rectangle alpha_bounds{5, 171, 150, 10};
    bool held = false, open = false;
    PopupMenu popup;
    PickerStatus status = PickerStatus::idle;
    HsvColor hsv;
    ColorClipboard* clipboard = &system_color_clipboard();

    void set_value(std::uint32_t color);
    PickerStatus region_at(Point cursor) const;
    void update_at(Point cursor, PickerStatus region);
    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void config_loaded() override;
    void reset_input() override;
    std::int32_t input_priority() const override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    static void menu_selected(Widget& owner, PopupMenu& popup, std::uint32_t index);
};

} // namespace skeet::ui
