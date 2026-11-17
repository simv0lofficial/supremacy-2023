#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/config.hpp"
#include "skeet/config_storage/native_blob_owner.hpp"
#include "skeet/menu_window.hpp"
#include <memory>
#include <variant>

namespace skeet::script {

// The native 28-byte record keeps widget/configuration pointers, a vector of
// Lua table references, and the byte distinguishing borrowed client controls.
// Configuration objects retain their concrete type and stable registered address.
using UiConfiguration =
    std::variant<std::monostate, std::unique_ptr<BooleanConfig>, std::unique_ptr<IntegerConfig>,
                 std::unique_ptr<ColorConfig>, std::unique_ptr<IntegerArrayConfig>,
                 std::unique_ptr<HotkeyConfig>, std::unique_ptr<ConfigBlob>,
                 std::unique_ptr<NativeConfigBlobOwner>>;

struct UiBinding {
    ui::Widget* widget = nullptr;
    ConfigDescriptor* configuration = nullptr;
    std::vector<int> callback_references;
    bool borrowed = false;
    std::unique_ptr<ui::Widget> owned_widget;
    UiConfiguration owned_configuration;

    UiBinding() = default;
    UiBinding(UiBinding&&) noexcept = default;
    UiBinding& operator=(UiBinding&&) noexcept = default;
    UiBinding(const UiBinding&) = delete;
    UiBinding& operator=(const UiBinding&) = delete;
};

class UiBindings {
public:
    struct CallbackRegistration {
        int reference;
        ui::WidgetCallback callback;
    };
    UiBindings() = default;
    UiBindings(const UiBindings&) = delete;
    UiBindings& operator=(const UiBindings&) = delete;
    UiBindings(UiBindings&&) = delete;
    UiBindings& operator=(UiBindings&&) = delete;
    ui::MenuWindow* window = nullptr;
    std::vector<UiBinding> records; // 0x43479760..64
    int values_reference = -1; // 0x4346D634: callback reference -> UI handle
    int callbacks_reference = -1; // 0x4346D630: callback reference -> Lua function

    // 0x43419FFC reuses only a borrowed-widget record, even when the same
    // widget also occurs in an owned record. Handles are zero-based.
    std::uint32_t reference(ui::Widget&);
    // Typed counterpart of record creation 0x433655E6. The category already
    // owns the visible element ordering; this transfers its dynamic lifetime.
    std::uint32_t adopt(std::unique_ptr<ui::Widget>, UiConfiguration = {});

    // 0x433F904C: replace callbacks on owned controls, append on borrowed ones.
    // The reference is allocated in callbacks_reference, not the Lua registry.
    bool set_callback(Bridge&, std::uint32_t handle, int function_index,
                      void* call_state = nullptr);
    // 0x43365BC4 allocates a button callback before appending its UI record.
    CallbackRegistration create_callback(Bridge&, std::uint32_t handle, int function_index,
                                         void* call_state = nullptr);
    // 0x4339CD77 plus its concrete WidgetCallback adapter 0x43368987.
    void dispatch_callback(Bridge&, int reference);
    void initialize(Bridge&); // table setup in 0x433FA380
    void clear_callbacks(Bridge&); // 0x433413B5
    void release_callbacks(Bridge&); // 0x433B86C2
    void destroy_bindings(); // 0x433ECDEE
    //433C8C11 releases callback-vector/record storage after destroy_bindings.
    // It never substitutes widget/configuration destruction or Lua unrefs.
    void finalize_record_storage();

private:
    struct CallbackContext {
        UiBindings* bindings;
        Bridge* bridge;
        int reference;
    };
    std::vector<std::unique_ptr<CallbackContext>> callback_contexts_;
    std::vector<std::uint8_t> callback_active_; // 0x43479784..88
    static void widget_changed(void*, ui::Widget&);
    void remove_widget_callback(UiBinding&, int);
};

} // namespace skeet::script
