#pragma once
#include "skeet/script/ui.hpp"

namespace skeet::input {
class Runtime;
}

namespace skeet::script {
class UiTypeStorage;

// Value/control half of the 24-entry UI module. UiModule owns this object and
// installs its closures into the same table as the constructors/reference.
class UiValues {
public:
    UiValues(Bridge& bridge, UiBindings& bindings, input::Runtime& input_runtime,
             UiTypeStorage& ui_types) noexcept
        : bridge_(bridge), bindings_(bindings), input_(input_runtime), types_(ui_types) {}
    UiValues(const UiValues&) = delete;
    UiValues& operator=(const UiValues&) = delete;
    void register_into(int table_index);
    int get(void*); // 433F2B4D
    int set(void*); // 433290A0
    int set_callback(void*); // 433F904C
    int set_visible(void*); // 433B1EEE
    int set_enabled(void*); // 433B5A87
    int name(void*); // 4338A6D6
    int is_menu_open(void*); // 433F0AC2
    int mouse_position(void*); // 43386DE8
    int menu_position(void*); // 433D6C58
    int menu_size(void*); // 4339FA45
    int update(void*); // 43368ADB
    int type(void*); // 4341F0BD

private:
    Bridge& bridge_;
    UiBindings& bindings_;
    // 43468560's bitmap reads the live key buffer43476428 on each hotkey get.
    input::Runtime& input_;
    UiTypeStorage& types_; //same43479670 storage initialized by ordinary433E495A
    UiBinding* checked_binding(void*, int minimum, std::uint32_t* handle = nullptr);
    ui::MenuWindow& window() const;
    int get_widget(void*, ui::Widget&);
    int set_widget(void*, ui::Widget&, int arguments);
    static int get_entry(void*);
    static int set_entry(void*);
    static int callback_entry(void*);
    static int visible_entry(void*);
    static int enabled_entry(void*);
    static int name_entry(void*);
    static int open_entry(void*);
    static int mouse_entry(void*);
    static int position_entry(void*);
    static int size_entry(void*);
    static int update_entry(void*);
    static int type_entry(void*);
};

} // namespace skeet::script
