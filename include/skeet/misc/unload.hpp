#pragma once
#include "skeet/config_storage/storage.hpp"
#include "skeet/config_storage/load_state.hpp"
#include "skeet/config_storage/config_stream_view.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/engine/unload.hpp"
#include <stdexcept>

namespace skeet::script {
class UiBindings;
}
namespace skeet::misc {
using UnloadTimerCallback = void(__stdcall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t);
// 0x433B0BFB is a separate native finalization stage, not an empty callback.
// Its concrete definition owns KillTimer, final destruction, and section unmap.
void __stdcall unload_timer_callback(void*, std::uint32_t, std::uintptr_t, std::uint32_t);

struct UnloadImports {
    std::uintptr_t (*set_timer)(std::uintptr_t, std::uintptr_t, std::uint32_t, UnloadTimerCallback);
    UnloadImports();
};
struct ConfigWidgetBinding {
    ui::Widget* widget = nullptr;
    ConfigDescriptor* descriptor = nullptr;
    bool disabled = false; // native binding +0x18
};
struct UnloadRequestGate {
    void* context = nullptr;
    bool (*allow)(void*) = nullptr;
};
class UnloadRuntime {
    script::UiBindings* source_ui_ = nullptr;
    const bool* source_preserve_toggle_ = nullptr;

public:
    explicit UnloadRuntime(ui::MenuWindow& window) : main_menu(window) {
        if (!main_menu.native_window)
            throw std::invalid_argument("Client unload requires the resolved main window");
    }
    // Production route borrows the actual dynamic Lua UI records. The plain
    // window constructor retains the explicit standalone binding-span route.
    UnloadRuntime(ui::MenuWindow& window, script::UiBindings& ui, const bool& preserve_toggle)
        : UnloadRuntime(window) {
        source_ui_ = &ui;
        source_preserve_toggle_ = &preserve_toggle;
        preserve_hotkey_toggle = reinterpret_cast<const std::uint8_t*>(&preserve_toggle);
    }
    script::UiBindings* source_ui() const noexcept {
        return source_ui_;
    }
    ui::MenuWindow& main_menu;
    ConfigLink* config_registry = nullptr;
    ConfigStreamView baseline_config = ConfigStreamView::unbound(); // live native 0x43478CB8
    const std::uint8_t* preserve_hotkey_toggle = config_loading_byte(); // native 0x43475A98
    ConfigStreamView current_config = ConfigStreamView::unbound(); // live script ShutdownState
    std::span<const ConfigWidgetBinding> config_bindings; // native 0x43479760..0x43479764
    input::Runtime* input = nullptr; // disabled is the shared DWORD 0x4347C9FC
    engine::UnloadRuntime* hooks = nullptr;
    UnloadImports imports;
    UnloadTimerCallback timer_callback = &unload_timer_callback;
    // Optional source startup barrier, evaluated before input/hooks change.
    UnloadRequestGate request_gate;

    void request(); // complete native 0x433A0637 coordination
    void refresh_bound_settings(); // complete native 0x433AB1C2 / 0x4337EABC
};
} // namespace skeet::misc
