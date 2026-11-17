#pragma once
#include "skeet/core.hpp"

namespace skeet::ui {
class MenuWindow;
struct WindowMessage;
struct WindowMessageHandler;

struct WindowClientBounds {
    std::int32_t left, top, right, bottom;
};
bool window_client_bounds(void*, std::uintptr_t window, WindowClientBounds& bounds);
bool window_screen_to_client(void*, std::uintptr_t window, Point& cursor);
struct WindowEventServices {
    void* context = nullptr;
    bool (*client_bounds)(void*, std::uintptr_t, WindowClientBounds&) = window_client_bounds;
    bool (*screen_to_client)(void*, std::uintptr_t, Point&) = window_screen_to_client;
};
struct WindowEventState {
    std::uint8_t window_size_locked = 1; // Shared native byte at 0x43467E4B, image default1.
};
inline WindowEventState& window_event_state() noexcept {
    static WindowEventState state;
    return state;
}

bool route_window_mouse(MenuWindow& window, WindowMessage& message);
bool decode_window_mouse(MenuWindow& window, WindowMessage& message);
void select_window_tab(MenuWindow& window, std::uint32_t requested);
void previous_window_tab(MenuWindow& window);
void next_window_focus(MenuWindow& window);
bool window_key_down(MenuWindow& window, WindowMessage& message);
bool window_key_up(MenuWindow& window, WindowMessage& message);
bool window_character(MenuWindow& window, WindowMessage& message);
bool window_mouse_move(MenuWindow& window, WindowMessage& message);
bool window_left_down(MenuWindow& window, WindowMessage& message);
bool window_left_up(MenuWindow& window, WindowMessage& message);
bool window_right_down(MenuWindow& window, WindowMessage& message);
bool window_right_up(MenuWindow& window, WindowMessage& message);
bool window_wheel(MenuWindow& window, WindowMessage& message);
std::span<const WindowMessageHandler> native_window_handlers() noexcept;
}
