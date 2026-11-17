#pragma once
#include "skeet/config.hpp"
#include "skeet/menu_window.hpp"
#include "skeet/engine/interface_call.hpp"
#include <stdexcept>

namespace skeet::input {
struct EngineInterfaces {
    void* engine_client = nullptr;
    void* hud = nullptr;
    engine::MemberFunction<void*, const char*> find_hud_element = nullptr;
    void* cursor_contexts = nullptr;
    void* input_system = nullptr;
    bool in_game() const;
    bool console_visible() const;
    bool chat_active() const;
};
bool is_typing_key(std::uint32_t key) noexcept;

class Runtime {
    std::int32_t standalone_disabled_ = 0;

public:
    Runtime() noexcept : disabled(standalone_disabled_) {}
    explicit Runtime(std::int32_t& disabled_word) noexcept : disabled(disabled_word) {}
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;
    EngineInterfaces interfaces;
    std::array<std::uint32_t, 8> keys{};
    std::uint16_t mouse_buttons = 0, captured_mouse_buttons = 0;
    RegisteredHotkey* menu_hotkey = nullptr; // Canonical43478DB8, packed word+10.
    // 0x4347C9FC: unload exchanges this word; input compares the whole word.
    std::int32_t& disabled;
    ui::MenuWindow* main_menu = nullptr;
    ui::MenuWindow* windows = nullptr;
    HotkeyLink* hotkey_registry = nullptr;
    std::uint32_t cursor_context = 0;

    InputBitmap bitmap() const noexcept {
        return {keys, 256};
    }
    void bind_configuration(ConfigRegistries& configuration, RegisteredHotkey& menu_key) {
        for (auto* entry = configuration.hotkeys.next; entry && entry != &configuration.hotkeys;
             entry = entry->next) {
            if (entry != &menu_key)
                continue;
            hotkey_registry = &configuration.hotkeys;
            menu_hotkey = &menu_key;
            return;
        }
        throw std::invalid_argument(
            "Menu hotkey must be registered in the canonical configuration registry");
    }
    void update_keys(const ui::InputMessage& message, bool consumed);
    bool dispatch(ui::WindowMessage& message);
    void enable_cursor();
    void release_cursor();
    void destroy_cursor();
    void toggle_menu();

private:
    void set_key(std::uint32_t key, bool pressed);
};
// Complete434928A2: registers43478DB8, clears callback/toggle, preserves bit0,
// and initializes Delete (0x2E) in hold mode. Call once during static startup.
void initialize_menu_hotkey(ConfigRegistries&, RegisteredHotkey&) noexcept;
}
