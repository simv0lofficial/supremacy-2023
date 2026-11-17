#pragma once
#include "gui_layout.hpp"
#include "render_queue.hpp"
#include "menu/window_events.hpp"
#include <array>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

namespace skeet::ui {

struct InputMessage {
    std::uint32_t message = 0;
    std::uint32_t parameter = 0;
    std::uint32_t key_data = 0;
};
struct SliderMetrics {
    std::int32_t minimum_width = 0;
    std::int32_t preferred_maximum_width = 0;
    std::int32_t category_padding = 0;
};
struct MenuInput {
    explicit MenuInput(std::uint32_t flags = 0) noexcept : mouse_flags(flags) {}
    MenuInput(std::uint32_t flags, std::uint8_t& layout_lock) noexcept
        : mouse_flags(flags), lock_category_layout(layout_lock) {}
    std::uint32_t mouse_flags = 0; // Native window +0x34: open, drag and resize state.
    std::uint32_t button_flags = 0; // Native window +0x38: mouse buttons, Shift and Ctrl.
    float scale = 1.0f;
    SliderMetrics slider_metrics{};
    // Category dragging and window resizing read the same native byte.
    std::uint8_t& lock_category_layout = window_event_state().window_size_locked;
};
struct WidgetIdentities {
    std::atomic<std::uint32_t> last{0};
    std::uint32_t allocate() noexcept {
        return last.fetch_add(1) + 1;
    }
};

class Widget;
struct WidgetCallback {
    void* context;
    void (*function)(void*, Widget&);
};
struct VisibilityLink {
    Widget* widget = nullptr;
    VisibilityLink* next = nullptr;
};

class Widget : public LayoutElement {
public:
    Widget();
    Widget(Widget* owner, std::u16string_view label, std::uint8_t type, bool has_value,
           WidgetIdentities& identities);
    virtual ~Widget() = default;
    Widget* parent = nullptr;
    MenuInput* menu_input = nullptr;
    bool enabled = true;
    bool hovered = false;
    bool focusable = true;
    std::uint32_t identity = 0;
    std::uint32_t text_flags = 0;
    std::uint32_t color = 0xc8ffffff;
    std::u16string name;
    std::vector<WidgetCallback> callbacks;

    void notify_callbacks();
    void set_visible(bool value);
    MenuInput* find_menu() const noexcept;
    void draw_label(RenderQueue& queue, Point& cursor, std::int32_t maximum_width) const;
    void request_focus();
    virtual void visibility_changed();
    virtual void on_layout() {}
    virtual void reset_input();
    virtual void value_changed();
    virtual bool hit_test(Point cursor) const;
    virtual bool mouse(const InputMessage& message, Point cursor);
    virtual bool keyboard(const InputMessage& message);
    virtual void config_loaded();
    virtual std::int32_t input_priority() const;
    virtual void resize(const SliderMetrics& metrics);
    virtual void dpi_changed(float scale, const SliderMetrics& metrics);
    virtual void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const;
};

class Label : public Widget {
public:
    Label();
    Label(Widget* owner, std::u16string_view text, WidgetIdentities& identities);
    bool hit_test(Point) const override {
        return false;
    } // 0x43336939
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};

class Checkbox : public Widget {
public:
    Checkbox();
    Checkbox(Widget* owner, std::u16string_view label, std::uint8_t* setting,
             WidgetIdentities& identities);
    std::uint8_t* value = nullptr;
    Point box_size{8, 8};
    std::int32_t text_padding = 20;
    bool held = false;
    VisibilityLink tied;

    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void set_value(bool checked);
    void value_changed() override;
    void config_loaded() override;
    void reset_input() override;
    void dpi_changed(float scale);
    void dpi_changed(float scale, const SliderMetrics&) override {
        dpi_changed(scale);
    }
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};

class Slider : public Widget {
public:
    Slider();
    Slider(Widget* owner, std::u16string_view label, std::int32_t lower, std::int32_t upper,
           std::int32_t* setting, bool show, std::uint32_t unit, float multiplier,
           WidgetIdentities& identities);
    std::int32_t* value = nullptr;
    Point track_size{158, 6};
    bool dragging = false;
    bool show_value = true;
    std::int32_t maximum = 100, minimum = 0;
    float display_scale = 1.0f;
    std::int32_t maximum_width = 200;
    std::uint32_t suffix = 0;
    std::uint32_t fill_color = 0xff2bca9f;
    std::unordered_map<std::uint32_t, std::wstring> value_labels;
    std::array<wchar_t, 64> value_text{};

    bool hit_test(Point cursor) const override;
    std::int32_t value_at(Point cursor) const noexcept;
    void set_value(std::int32_t requested);
    void value_changed() override;
    void config_loaded() override;
    void update_value_text();
    bool mouse(const InputMessage& message, Point cursor) override;
    bool keyboard(const InputMessage& message) override;
    void reset_input() override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    void draw_value(RenderQueue& queue, std::int32_t x, std::int32_t y, std::int32_t fill_width,
                    std::int32_t height) const;
};

} // namespace skeet::ui
