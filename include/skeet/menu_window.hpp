#pragma once
#include "menu_tree.hpp"
#include "menu/window_events.hpp"

namespace skeet::ui {
class MenuWindow;
struct WindowMessage {
    std::uintptr_t window = 0;
    InputMessage input;
};
struct WindowNotification {
    std::uint32_t key;
    void (*callback)(MenuWindow&, std::uint32_t);
    WindowNotification* next = nullptr;
};
struct WindowMessageHandler {
    std::uint32_t key;
    bool (*callback)(MenuWindow&, WindowMessage&);
    WindowMessageHandler* next = nullptr;
};
std::uint32_t window_ticks(void* context);

class MenuWindow {
public:
    MenuWindow() noexcept : MenuWindow(window_event_state()) {}
    explicit MenuWindow(WindowEventState& state) noexcept
        : event_state(&state), input(0, state.window_size_locked) {}
    MenuWindow* next = nullptr;
    Point default_size{};
    Point position{};
    std::uintptr_t native_window = 0;
    Point cursor{}, drag_anchor{};
    WindowEventServices event_services{};
    WindowEventState* event_state = &window_event_state();
    MenuGeometry geometry;
    MenuInput input;
    // Widgets reach the same +0x38 word through their owning tab's menu input.
    std::uint32_t& button_flags = input.button_flags;
    std::vector<Tab*> tabs;
    Menu navigation{};
    std::uint32_t transition_started = 0;
    std::int32_t transition_state = 0;
    std::uint32_t background_texture = UINT32_MAX;
    float opacity = 1.0f;
    // Each bucket array includes its native, non-null end marker as the final
    // slot. Nodes are non-owning; equal notification keys form a contiguous run.
    std::span<WindowNotification*> notifications;
    std::span<WindowMessageHandler*> message_handlers;
    void* clock_context = nullptr;
    std::uint32_t (*read_ticks)(void*) = window_ticks;

    void update_dpi(float scale);
    bool dispatch(WindowMessage& message);
    void open();
    void update_transition(RenderQueue& queue);
    void draw(RenderQueue& queue, std::uint32_t accent);
};

struct DpiResourceCallback {
    void* context;
    void (*refresh)(void*);
};
struct GuiDpiState {
    // Native table at 0x434745CC. Selection is refreshed by its existing widget
    // before the current value is read, including when the scale is unchanged.
    static constexpr std::array<float, 5> scales{1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
    // The combo box, persistence restore and refresh all access 43475A94.
    // Keeping a copy here loses config changes that do not invoke the callback.
    std::int32_t& selection;
    float active_scale = 1.0f;
    Widget* setting_widget = nullptr;
    DpiResourceCallback font_resources{};
    MenuWindow* windows = nullptr;

    explicit GuiDpiState(std::int32_t& setting) noexcept : selection(setting) {}
    void refresh(bool force);
};
}
