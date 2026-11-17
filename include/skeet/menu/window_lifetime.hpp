#pragma once
#include "skeet/menu_window.hpp"
#include "skeet/config_storage/owner.hpp"

namespace skeet::ui {
// Native434798CC is the shared preset-store NT path, freed by433E71FA.
using WindowTitleString = skeet::NativeConfigPath;
struct WindowLifetimeImports {
    using FreeUnicodeString = void(__stdcall*)(WindowTitleString*);
    FreeUnicodeString free_unicode_string;
};
WindowLifetimeImports native_window_lifetime_imports();
// Own the callback nodes behind MenuWindow's non-owning native bucket views.
// The fixed nine registrations in 0x43485772 finish with eleven buckets.
class WindowCallbackStorage {
public:
    void initialize(MenuWindow& window);
    void destroy(MenuWindow& window) noexcept;

private:
    std::vector<WindowMessageHandler> handlers_;
    std::vector<WindowMessageHandler*> handler_buckets_;
    WindowMessageHandler handler_end_{};
    std::array<WindowNotification*, 2> notification_buckets_{};
    WindowNotification notification_end_{};
};

void initialize_menu_window(MenuWindow& window, WindowCallbackStorage& callbacks,
                            std::uintptr_t native_window, Point size);
void destroy_menu_window(MenuWindow& window, WindowCallbackStorage& callbacks) noexcept;
MenuWindow* delete_menu_window(MenuWindow* window, WindowCallbackStorage& callbacks,
                               std::uint8_t flags) noexcept;
void destroy_main_menu_window(MenuWindow& window, WindowCallbackStorage& callbacks,
                              WindowTitleString& title,
                              const WindowLifetimeImports& imports) noexcept;
MenuWindow* delete_main_menu_window(MenuWindow* window, WindowCallbackStorage& callbacks,
                                    WindowTitleString& title, const WindowLifetimeImports& imports,
                                    std::uint8_t flags) noexcept;
}
