// Menu Component: form
#include "skeet/menu_window.hpp"
#include "skeet/popup_menu.hpp"
#include <algorithm>
#include <bit>
#include <windows.h>
#include <stdexcept>
#include "skeet/menu/window_render.hpp"
#include "skeet/menu/window_events.hpp"


// ============================================================================
// Subcomponent: window_layout.cpp
// ============================================================================

namespace skeet::ui {
namespace {
using i32 = std::int32_t;
inline i32 add(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) + std::uint32_t(b));
}
inline i32 sub(i32 a, i32 b) noexcept {
    return std::bit_cast<i32>(std::uint32_t(a) - std::uint32_t(b));
}
}
void MenuWindow::update_dpi(float scale) { // 0x433B23FC
    const auto minimum = scale_size(default_size, scale);
    const auto previous = size_for_persistence();
    const auto scaled = scale_size(previous, scale / input.scale);
    preferred_size_ = {std::max(minimum.x, scaled.x), std::max(minimum.y, scaled.y)};
    geometry.size = preferred_size_;
    input.scale = scale;
    input.slider_metrics = {round_layout_coordinate(40.0f * scale),
                            round_layout_coordinate(200.0f * scale),
                            round_layout_coordinate(100.0f * scale)};
    if (tab_context_menu) {
        tab_context_menu->reset_input();
        tab_context_menu->dpi_changed(scale, input.slider_metrics);
    }
    auto y = 20;
    for (auto*& tab : std::span(tabs)) {
        tab->dpi_changed(scale, input.slider_metrics);
        tab->geometry.bounds.y = y;
        tab->geometry.bounds.x = add(tab->geometry.horizontal_offset, 6);
        y = add(y, tab->geometry.bounds.height);
    }
    for (auto*& tab : std::span(tabs)) {
        tab->layout_categories();
        tab->reflow_categories();
        tab->resize(input.slider_metrics);
    }
    const auto viewport = viewport_;
    viewport_ = {};
    update_viewport(viewport);
}
void MenuWindow::resize(Point size) {
    preferred_size_ = size;
    resize_layout(size);
}
void MenuWindow::resize_layout(Point size) {
    if (size.x == geometry.size.x && size.y == geometry.size.y)
        return;
    geometry.size = size;
    for (auto* tab : tabs)
        tab->resize(input.slider_metrics);
}
void MenuWindow::move(Point next) noexcept {
    position = next;
    centered_ = false;
}
void MenuWindow::restore_size(Point saved, Point viewport) {
    const auto minimum = scale_size(default_size, input.scale);
    // Older source builds wrote window coordinates into these size records.
    if (saved.x < minimum.x || saved.y < minimum.y)
        return;
    resize(saved);
    viewport_ = {};
    update_viewport(viewport);
}
Point MenuWindow::size_for_persistence() const noexcept {
    return preferred_size_.x > 0 && preferred_size_.y > 0 ? preferred_size_ : geometry.size;
}
bool MenuWindow::update_viewport(Point viewport) {
    // Minimized/uninitialized windows must not shrink saved dimensions or
    // consume the first-launch centering request.
    if (viewport.x <= 0 || viewport.y <= 0)
        return false;
    if (!startup_placement_pending && viewport.x == viewport_.x && viewport.y == viewport_.y)
        return true;
    viewport_ = viewport;
    const auto minimum = scale_size(default_size, input.scale);
    const auto preferred = size_for_persistence();
    resize_layout({std::clamp(preferred.x, minimum.x, std::max(viewport.x, minimum.x)),
                   std::clamp(preferred.y, minimum.y, std::max(viewport.y, minimum.y))});
    const Point limit{std::max(0, viewport.x - geometry.size.x),
                      std::max(0, viewport.y - geometry.size.y)};
    if (startup_placement_pending)
        centered_ = true;
    if (centered_)
        position = {limit.x / 2, limit.y / 2};
    else
        position = {std::clamp(position.x, 0, limit.x), std::clamp(position.y, 0, limit.y)};
    startup_placement_pending = false;
    return true;
}
void MenuWindow::refresh_viewport() {
    WindowClientBounds bounds{};
    if (event_services.client_bounds(event_services.context, native_window, bounds))
        update_viewport({bounds.right - bounds.left, bounds.bottom - bounds.top});
}
void MenuWindow::finish_resize() {
    const bool changed = (input.mouse_flags & 8u) && (geometry.size.x != resize_start_size.x ||
                                                      geometry.size.y != resize_start_size.y);
    input.mouse_flags &= ~12u;
    button_flags &= ~1u;
    if (changed && resize_finished.save)
        resize_finished.save(resize_finished.context);
}
void GuiDpiState::refresh(bool force) { // 0x4339F0AD
    if (setting_widget)
        setting_widget->config_loaded();
    const auto requested = scales.at(selection);
    if (!force && requested == active_scale)
        return;
    active_scale = requested;
    if (font_resources.context)
        font_resources.refresh(font_resources.context);
    for (auto* window = windows; window; window = window->next)
        window->update_dpi(active_scale);
}
}

// ============================================================================
// Subcomponent: window_input.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::ui {
namespace {
template <class Handler>
Handler* find_handler(std::span<Handler*> buckets, std::uint32_t key, std::size_t& bucket) {
    if (buckets.size() < 2 || !buckets.back())
        throw std::logic_error("Window callback table requires buckets and its native end marker");
    bucket = key % (buckets.size() - 1);
    for (auto* handler = buckets[bucket]; handler; handler = handler->next)
        if (handler->key == key)
            return handler;
    bucket = buckets.size() - 1;
    return buckets.back();
}
}
std::uint32_t window_ticks(void*) {
    return GetTickCount();
}

bool MenuWindow::dispatch(WindowMessage& message) { // 0x433BA305
    if (message.input.message == 0xa090) {
        std::size_t bucket = 0;
        auto* handler = find_handler(notifications, message.input.parameter, bucket);
        if (handler == notifications.back())
            return true;
        auto* end = handler->next;
        while (end && end->key == message.input.parameter)
            end = end->next;
        auto end_bucket = bucket;
        while (!end)
            end = notifications[++end_bucket]; // 0x43341DC7
        while (handler != end) {
            handler->callback(*this, message.input.key_data);
            handler = handler->next;
            while (!handler)
                handler = notifications[++bucket];
        }
        return true;
    }
    if (!(input.mouse_flags & 1) || transition_state != 0)
        return false;
    auto* focused = tabs.at(navigation.active_tab)->focused_widget();
    if (focused &&
        (focused->kind == 9 || focused->kind == 5 || focused->kind == 10 || focused->kind == 6))
        if (focused->keyboard(message.input))
            return true;
    std::size_t bucket = 0;
    auto* handler = find_handler(message_handlers, message.input.message, bucket);
    if (handler == message_handlers.back())
        return false;
    return handler->callback(*this, message);
}
void MenuWindow::open() { // 0x433AE63F
    if (input.mouse_flags & 1)
        return;
    input.mouse_flags |= 1;
    if (!tabs.empty())
        tabs.at(navigation.active_tab)->activate();
    transition_started = read_ticks(clock_context);
    transition_state = 2;
}
}

// ============================================================================
// Subcomponent: window_render.cpp
// ============================================================================

namespace skeet::ui {
void draw_window_frame(RenderQueue& queue, const WindowRenderState& window) { // 0x433AD3F2
    const auto& b = window.bounds;
    queue.rectangle_outline({b.x, b.y, b.width, b.height, 0xff0c0c0c}, 1);
    queue.rectangle_outline(
        {add(b.x, 1), add(b.y, 1), sub(b.width, 2), sub(b.height, 2), 0xff3c3c3c}, 1);
    queue.rectangle({add(b.x, 2), add(b.y, 2), sub(b.width, 4), sub(b.height, 4), 0xff282828});
    queue.rectangle_outline(
        {add(b.x, 5), add(b.y, 5), sub(b.width, 10), sub(b.height, 10), 0xff3c3c3c}, 1);
    queue.rectangle({add(b.x, 6), add(b.y, 6), sub(b.width, 12), sub(b.height, 12), 0xff0c0c0c});
    if (window.background_texture != UINT32_MAX)
        queue.texture({add(b.x, 6), add(b.y, 10), window.background_texture, 0xffffffff, 0, 0,
                       sub(b.width, 12), sub(b.height, 16), 0, 1.0f});
}
void draw_window_accent(RenderQueue& queue, const WindowRenderState& window) { // 0x4338490E
    const auto x = add(window.bounds.x, 7), y = add(window.bounds.y, 7);
    const auto width = sub(window.bounds.width, 14);
    const auto first_width = width / 2;
    queue.horizontal_gradient({x, y, first_width, 2, 0xffdab137, 0xffcd46cc});
    queue.horizontal_gradient(
        {add(x, first_width), y, sub(width, first_width), 2, 0xffcd46cc, 0xff35e3cc});
    queue.rectangle({x, add(y, 1), width, 2, 0x77000000});
}
void draw_window_decorations(RenderQueue& queue, const WindowRenderState& window,
                             const Rectangle& active_tab) { // 0x433D2AB3
    draw_window_frame(queue, window);
    const auto& b = window.bounds;
    const auto x = add(b.x, 6), y = add(b.y, 10);
    const auto width = active_tab.width, top_height = sub(active_tab.y, 10);
    const auto black_x = add(x, sub(width, 2)), border_x = add(x, sub(width, 1));
    if (add(top_height, add(active_tab.height, 20)) < b.height) {
        queue.rectangle({x, y, width, top_height, 0xff0c0c0c});
        queue.rectangle({black_x, y, 1, sub(top_height, 1), 0xff000000});
        queue.rectangle({border_x, y, 1, top_height, 0xff282828});
        queue.rectangle({x, add(y, sub(top_height, 2)), sub(width, 1), 1, 0xff000000});
        queue.rectangle({x, add(y, sub(top_height, 1)), width, 1, 0xff282828});
        const auto bottom_y = add(add(y, top_height), active_tab.height);
        const auto bottom_height = add(sub(b.height, bottom_y), sub(b.y, 6));
        queue.rectangle({x, bottom_y, width, bottom_height, 0xff0c0c0c});
        queue.rectangle({black_x, add(bottom_y, 1), 1, sub(bottom_height, 1), 0xff000000});
        queue.rectangle({border_x, bottom_y, 1, bottom_height, 0xff282828});
        queue.rectangle({x, bottom_y, width, 1, 0xff282828});
        queue.rectangle({x, add(bottom_y, 1), sub(width, 1), 1, 0xff000000});
    } else {
        const auto height = add(sub(b.height, y), sub(b.y, 6));
        queue.rectangle({x, y, width, height, 0xff0c0c0c});
        queue.rectangle({black_x, add(y, 1), 1, sub(height, 1), 0xff000000});
        queue.rectangle({border_x, y, 1, height, 0xff282828});
    }
    draw_window_accent(queue, window);
}
void MenuWindow::update_transition(RenderQueue& queue) { // 0x43384045
    if (transition_state == 2) {
        const auto elapsed = read_ticks(clock_context) - transition_started;
        opacity = static_cast<float>(elapsed) / 150.0f;
        if (opacity > 1.0f) {
            transition_state = 0;
            opacity = 1.0f;
        }
    } else if (transition_state == 1) {
        if (tab_context_menu)
            tab_context_menu->reset_input();
        const auto elapsed = read_ticks(clock_context) - transition_started;
        auto progress = static_cast<float>(elapsed) / 150.0f;
        if (progress > 1.0f) {
            finish_resize();
            transition_state = 0;
            opacity = 1.0f;
            auto flags = input.mouse_flags;
            if (!tabs.empty()) {
                for (auto* tab : std::span(tabs))
                    tab->reset_input();
                flags = input.mouse_flags;
                progress = opacity;
            } else
                progress = 1.0f;
            input.mouse_flags = flags & ~std::uint32_t(1);
        }
        opacity = 1.0f - progress;
    } else
        opacity = 1.0f;
    queue.set_alpha(opacity);
}
void MenuWindow::draw(RenderQueue& queue, std::uint32_t accent) { // 0x433AC5E2
    update_transition(queue);
    if (!(input.mouse_flags & 1))
        return;
    const WindowRenderState state{{position.x, position.y, geometry.size.x, geometry.size.y},
                                  background_texture};
    draw_window_decorations(queue, state, tabs.at(navigation.active_tab)->geometry.bounds);
    for (auto* tab : std::span(tabs))
        if (tab->visible)
            tab->draw(queue, position, accent);
    if (tab_context_menu && tab_context_menu->open && tab_context_menu->parent->visible) {
        const auto& bounds = tab_context_menu->parent->geometry.bounds;
        tab_context_menu->draw(queue, {add(position.x, bounds.x), add(position.y, bounds.y)}, accent);
    }
}
}

// ============================================================================
// Subcomponent: window_events.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::ui {
namespace {
Point packed_cursor(std::uint32_t value) noexcept {
    return {std::bit_cast<std::int16_t>(std::uint16_t(value)),
            std::bit_cast<std::int16_t>(std::uint16_t(value >> 16))};
}
bool inside(const MenuWindow& window) noexcept {
    return contains(
        {window.position.x, window.position.y, window.geometry.size.x, window.geometry.size.y},
        window.cursor);
}
Tab& active(MenuWindow& window) {
    return *window.tabs.at(window.navigation.active_tab);
}
void clear_text_focus(MenuWindow& window) { // 0x4332A762
    auto& tab = active(window);
    auto* focused = tab.focused_widget();
    if (focused && (focused->kind == 10 || focused->kind == 6))
        tab.focus_widget(nullptr);
}
void modifier(MenuWindow& window, std::uint32_t key, bool down) noexcept {
    std::uint32_t mask = 0;
    if (key == 0x10 || key == 0xa0 || key == 0xa1)
        mask = 4;
    else if (key == 0x11 || key == 0xa2 || key == 0xa3)
        mask = 8;
    if (down)
        window.button_flags |= mask;
    else
        window.button_flags &= ~mask;
}
bool tab_context_input(MenuWindow& window, const InputMessage& message, Point cursor) {
    auto* popup = window.tab_context_menu;
    if (!popup)
        return false;
    if (popup->open) {
        if (popup->mouse(message, cursor))
            return true;
        if (message.message == WM_LBUTTONDOWN || message.message == WM_RBUTTONUP)
            popup->reset_input();
        else if (popup->hit_test(cursor))
            return true;
    }
    if (message.message != WM_RBUTTONUP || !inside(window))
        return false;
    for (auto* tab : window.tabs) {
        if (!tab->visible || !tab->hit_test(cursor))
            continue;
        active(window).reset_input();
        popup->parent = tab;
        auto& rect = popup->geometry.bounds;
        rect.x = std::clamp(cursor.x, 0, std::max(0, window.geometry.size.x - rect.width)) -
                 tab->geometry.bounds.x;
        rect.y = std::clamp(cursor.y, 0, std::max(0, window.geometry.size.y - rect.height)) -
                 tab->geometry.bounds.y;
        popup->open = true;
        return true;
    }
    return false;
}
}
bool window_client_bounds(void*, std::uintptr_t window, WindowClientBounds& bounds) {
    RECT native{};
    if (IsIconic(reinterpret_cast<HWND>(window)) ||
        !GetClientRect(reinterpret_cast<HWND>(window), &native))
        return false;
    bounds = {native.left, native.top, native.right, native.bottom};
    return true;
}
bool window_screen_to_client(void*, std::uintptr_t window, Point& cursor) {
    POINT native{cursor.x, cursor.y};
    if (!ScreenToClient(reinterpret_cast<HWND>(window), &native))
        return false;
    cursor = {native.x, native.y};
    return true;
}
bool route_window_mouse(MenuWindow& window, WindowMessage& message) { // 0x4334EAE8
    const Point relative{sub(window.cursor.x, window.position.x),
                         sub(window.cursor.y, window.position.y)};
    if (tab_context_input(window, message.input, relative))
        return true;
    bool handled = false;
    const auto tabs = std::span(window.tabs);
    for (auto i = tabs.size(); i != 0;) {
        auto* tab = tabs[--i];
        if (tab->visible && tab->mouse(message.input, relative))
            handled = true;
    }
    return handled;
}
bool decode_window_mouse(MenuWindow& window, WindowMessage& message) { // 0x43398693
    window.cursor = packed_cursor(message.input.key_data);
    return route_window_mouse(window, message);
}
void select_window_tab(MenuWindow& window, std::uint32_t requested) { // 0x433B75D3
    if (requested == window.navigation.active_tab || requested >= window.tabs.size())
        return;
    if (window.tab_context_menu)
        window.tab_context_menu->reset_input();
    active(window).reset_input();
    window.navigation.active_tab = requested;
    active(window).activate();
}
void previous_window_tab(MenuWindow& window) { // 0x433FF5EE
    const auto tabs = std::span(window.tabs);
    auto selected = window.navigation.active_tab;
    for (std::size_t tried = 0; tried < tabs.size(); ++tried) {
        selected = selected ? selected - 1 : std::uint32_t(tabs.size() - 1);
        if (tabs[selected]->visible) {
            select_window_tab(window, selected);
            return;
        }
    }
}
void next_window_focus(MenuWindow& window) { // 0x433CBE38
    auto& tab = active(window);
    auto* focused = tab.focused_widget();
    bool after = focused == nullptr;
    for (auto* category : std::span(tab.categories))
        for (auto* child : std::span(category->elements)) {
            if (after && child->focusable && (child->kind == 10 || child->kind == 6)) {
                child->request_focus();
                return;
            }
            if (child == focused)
                after = true;
        }
    if (!focused)
        return;
    for (auto* category : std::span(tab.categories))
        for (auto* child : std::span(category->elements)) {
            if (child == focused)
                return;
            if (child->focusable && (child->kind == 10 || child->kind == 6)) {
                child->request_focus();
                return;
            }
        }
}
bool window_key_down(MenuWindow& window, WindowMessage& message) { // 0x433B6EEC
    const auto key = message.input.parameter;
    if (key == VK_ESCAPE && window.tab_context_menu && window.tab_context_menu->open) {
        window.tab_context_menu->reset_input();
        return true;
    }
    modifier(window, key, true);
    if (key == 9) {
        if (window.button_flags & 8) {
            if (window.button_flags & 4)
                previous_window_tab(window);
            else {
                const auto tabs = std::span(window.tabs);
                auto selected = window.navigation.active_tab;
                for (std::size_t tried = 0; tried < tabs.size(); ++tried) {
                    selected = std::uint32_t(selected + 1) % std::uint32_t(tabs.size());
                    if (tabs[selected]->visible) {
                        select_window_tab(window, selected);
                        break;
                    }
                }
            }
            return true;
        }
        next_window_focus(window);
    }
    return window_character(window, message);
}
bool window_key_up(MenuWindow& window, WindowMessage& message) { // 0x433CF538
    modifier(window, message.input.parameter, false);
    return false;
}
bool window_character(MenuWindow& window, WindowMessage& message) { // 0x433EB7BA
    if (auto* focused = active(window).focused_widget())
        return focused->keyboard(message.input);
    return false;
}
bool window_left_down(MenuWindow& window, WindowMessage& message) { // 0x43327C83
    window.button_flags |= 1;
    if (decode_window_mouse(window, message))
        return true;
    if (!inside(window)) {
        clear_text_focus(window);
        return false;
    }
    for (auto* tab : std::span(window.tabs))
        tab->reset_input();
    clear_text_focus(window);
    const auto grip = round_layout_coordinate(window.input.scale * 15.0f);
    const bool resize =
        !window.event_state->window_size_locked && !(window.input.mouse_flags & 0x10) &&
        sub(window.cursor.x, window.position.x) > sub(window.geometry.size.x, grip) &&
        sub(window.cursor.y, window.position.y) > sub(window.geometry.size.y, grip);
    window.input.mouse_flags |= resize ? 8 : 4;
    window.drag_anchor = window.cursor;
    if (resize)
        window.resize_start_size = window.geometry.size;
    return true;
}
bool window_left_up(MenuWindow& window, WindowMessage& message) { // 0x433EE1F2
    window.finish_resize();
    const auto handled = decode_window_mouse(window, message);
    window.input.mouse_flags &= ~12u;
    window.button_flags &= ~1u;
    if (auto* focused = active(window).focused_widget(); focused && focused->kind == 5)
        static_cast<Slider*>(focused)->dragging = false;
    return handled;
}
bool window_right_down(MenuWindow& window, WindowMessage& message) { // 0x4334C26E
    window.button_flags |= 2;
    return decode_window_mouse(window, message) || inside(window);
}
bool window_right_up(MenuWindow& window, WindowMessage& message) { // 0x433D0B03
    window.button_flags &= ~2u;
    return decode_window_mouse(window, message);
}
bool window_wheel(MenuWindow& window, WindowMessage& message) { // 0x43408192
    window.cursor = packed_cursor(message.input.key_data);
    auto converted = window.cursor;
    if (window.event_services.screen_to_client(window.event_services.context, window.native_window,
                                               converted))
        window.cursor = converted;
    return route_window_mouse(window, message) || inside(window);
}
bool window_mouse_move(MenuWindow& window, WindowMessage& message) { // 0x4339E90B
    const auto handled = decode_window_mouse(window, message);
    if (window.input.mouse_flags & 4) {
        Point next{add(sub(window.position.x, window.drag_anchor.x), window.cursor.x),
                   add(sub(window.position.y, window.drag_anchor.y), window.cursor.y)};
        WindowClientBounds client{};
        if (window.event_services.client_bounds(window.event_services.context, window.native_window,
                                                client)) {
            const Point minimum{window.geometry.size.x / -3, window.geometry.size.y / -3};
            const Point maximum{sub(client.right, window.geometry.size.x / 3),
                                sub(client.bottom, window.geometry.size.y / 3)};
            if (next.x < minimum.x)
                next.x = minimum.x;
            else if (next.x > maximum.x)
                next.x = maximum.x;
            if (next.y < minimum.y)
                next.y = minimum.y;
            else if (next.y > maximum.y)
                next.y = maximum.y;
        }
        if (next.x != window.position.x || next.y != window.position.y) {
            window.move(next);
            window.drag_anchor = window.cursor;
        }
    } else if ((window.input.mouse_flags & 8) && (window.cursor.x != window.drag_anchor.x ||
                                                  window.cursor.y != window.drag_anchor.y)) {
        auto size = window.geometry.size;
        const Point next{add(size.x, sub(window.cursor.x, window.drag_anchor.x)),
                         add(size.y, sub(window.cursor.y, window.drag_anchor.y))};
        const auto minimum = scale_size(window.default_size, window.input.scale);
        if (next.x >= minimum.x) {
            size.x = next.x;
            window.drag_anchor.x = window.cursor.x;
        }
        if (next.y >= minimum.y) {
            size.y = next.y;
            window.drag_anchor.y = window.cursor.y;
        }
        window.resize(size);
    }
    const auto buttons = message.input.parameter & 15;
    const auto changed = window.button_flags ^ buttons;
    if (changed) {
        window.button_flags = buttons;
        if (changed & 1) {
            if (buttons & 1)
                window_left_down(window, message);
            else
                window_left_up(window, message);
        }
        if (changed & 2) {
            if (buttons & 2)
                window_right_down(window, message);
            else
                window_right_up(window, message);
        }
    }
    return handled || inside(window);
}
std::span<const WindowMessageHandler> native_window_handlers() noexcept {
    // Registrations in 0x4348580B..0x43485994; all entries use the native message identity.
    static const std::array<WindowMessageHandler, 9> handlers{{{0x100, window_key_down},
                                                               {0x101, window_key_up},
                                                               {0x102, window_character},
                                                               {0x200, window_mouse_move},
                                                               {0x201, window_left_down},
                                                               {0x202, window_left_up},
                                                               {0x204, window_right_down},
                                                               {0x205, window_right_up},
                                                               {0x20a, window_wheel}}};
    return handlers;
}
}
