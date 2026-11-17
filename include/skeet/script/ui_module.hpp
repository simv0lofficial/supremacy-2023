#pragma once
#include "skeet/script/ui.hpp"
#include "skeet/script/ui_values.hpp"
#include <array>

namespace skeet::input {
class Runtime;
}

namespace skeet::script {

// C closures retain this concrete owner as their light-userdata upvalue.
// It must remain alive until the corresponding Lua state is closed.
class UiModule {
public:
    UiModule(Bridge& bridge, UiBindings& bindings, ConfigRegistries& configuration,
             ui::WidgetIdentities& identities, input::Runtime& input_runtime,
             UiTypeStorage& ui_types)
        : bridge_(bridge), bindings_(bindings), configuration_(configuration),
          identities_(identities), types_(ui_types),
          values_(bridge, bindings, input_runtime, ui_types) {}
    UiModule(const UiModule&) = delete;
    UiModule& operator=(const UiModule&) = delete;
    // 0x433FA380: creates both registry tables, reserves records, and installs
    // the exact 24-entry UI catalog through this owner and its UiValues member.
    void register_exports();
    // Native Lua C entry points 0x433C905F / 0x4341A695.
    int new_checkbox(void* state);
    int new_slider(void* state);
    int new_combobox(void* state);
    int new_multiselect(void* state);
    int new_hotkey(void* state);
    int new_button(void* state);
    int new_color_picker(void* state);
    int new_textbox(void* state);
    int new_listbox(void* state);
    int new_string(void* state);
    int new_label(void* state);
    int reference(void* state);
    UiBindings& bindings() noexcept {
        return bindings_;
    }
    UiValues& values() noexcept {
        return values_;
    }

private:
    struct Target {
        ui::Tab* tab = nullptr;
        ui::Category* category = nullptr;
        std::array<char16_t, 128> label{};
        std::uint32_t hash = 0;
    };
    Bridge& bridge_;
    UiBindings& bindings_;
    ConfigRegistries& configuration_;
    ui::WidgetIdentities& identities_;
    UiTypeStorage& types_;
    UiValues values_;
    int parse_target(void* state, Target&); // 0x4341E5EF
    void attach(Target&, ui::Widget&); // 0x4336C436
    int finish(void*, Target&, ui::Widget&);
    int validate_options(void*, bool empty_table_is_argument_error);
    static UiModule& from_state(void*);
    static int checkbox_entry(void*);
    static int slider_entry(void*);
    static int combobox_entry(void*);
    static int multiselect_entry(void*);
    static int hotkey_entry(void*);
    static int button_entry(void*);
    static int color_picker_entry(void*);
    static int textbox_entry(void*);
    static int listbox_entry(void*);
    static int string_entry(void*);
    static int label_entry(void*);
    static int reference_entry(void*);
};

} // namespace skeet::script
