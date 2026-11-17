#pragma once
#include "skeet/esp/effects_config.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <array>

namespace skeet {
struct MovementCommandOwner;
}
namespace skeet::effects {
struct ViewOffsets {
    std::uint32_t life_state; // 0x9D920E35
    std::uint32_t scoped; // 0xCACAEECC
    std::uint32_t active_weapon; // 0xCD812654
    std::uint32_t fov; // 0xFBE8C5EA
    std::uint32_t scope_transition_35b5b2ec;
    std::uint32_t shake_angles_5ae32976;
    std::uint32_t recoil_angles_1bae1168;
};
struct ViewBindings {
private:
    void* standalone_entities_ = nullptr;
    void* standalone_panels_ = nullptr;
    void* standalone_input_ = nullptr;
    void* standalone_prediction_ = nullptr;
    void* standalone_recoil_ = nullptr;
    void* standalone_surface_ = nullptr;

public:
    ViewBindings() noexcept
        : entity_list(standalone_entities_), hud_panels(standalone_panels_),
          input(standalone_input_), prediction(standalone_prediction_),
          recoil_scale(standalone_recoil_), panel_surface(standalone_surface_) {}
    ViewBindings(void* entities, void* panels, void* input_object, void* prediction_object,
                 void* recoil, const std::uint8_t* disabled, const std::uint8_t* calculating,
                 void* surface = nullptr) noexcept
        : ViewBindings() {
        entity_list = entities;
        hud_panels = panels;
        input = input_object;
        prediction = prediction_object;
        standalone_recoil_ = recoil;
        suspended = disabled;
        calculating_view = calculating;
        panel_surface = surface;
    }
    explicit ViewBindings(engine::NativeDataImage&);
    ViewBindings(const ViewBindings&) = delete;
    ViewBindings& operator=(const ViewBindings&) = delete;
    void*& entity_list; //live4346A890
    void*& hud_panels; //live4346A44C
    void*& input; //live4346A894
    void*& prediction; //live43469264; byte+8
    engine::NativeConVarReference recoil_scale; //live4346A534 XOR B739EB42
    const std::uint8_t* suspended = nullptr; // 0x43468E58
    const std::uint8_t* calculating_view = nullptr; // 0x43475BD1
    void*& panel_surface; //live4346712C; GetPanel(2) through slot1
};
struct SavedRecoilAngles {
    void* shake = nullptr;
    void* all = nullptr;
    std::array<std::uint32_t, 3> previous_shake{}, previous_all{};
};
struct SavedViewHeight {
    void* address = nullptr;
    std::uint32_t value = 0;
};
class ViewDuckTarget {
public:
    ViewDuckTarget(const float* target) noexcept : target_(target), owner_(nullptr) {}
    explicit ViewDuckTarget(MovementCommandOwner* const& owner) noexcept
        : target_(nullptr), owner_(&owner) {}
    const float* get() const noexcept;

private:
    const float* target_;
    MovementCommandOwner* const* owner_;
};
struct ViewDuckBindings {
    ViewDuckTarget duck_target; // live movement owner434759B0, null if absent
    const std::uint8_t& enabled; //43467CEC
    const std::uint32_t& command_flags; //434759C0
    const HotkeyBinding& duck_key; //43467C8C, activation mode43467CA0
    InputBitmap input;
    float& previous_duck; //4346D654
    std::uint32_t duck_amount, view_offset; //3309C0CF /C246315E
};

class ViewRuntime {
public:
    ViewRuntime(const esp::EffectsSettings& settings, ViewBindings& interfaces, ViewOffsets offsets)
        : settings(settings), interfaces(interfaces), offsets(offsets) {}
    const esp::EffectsSettings& settings;
    ViewBindings& interfaces;
    ViewOffsets offsets;
    std::int32_t scope_panel = -1; // scope owner's +0x28

    void initialize_scope_panel(); // complete 0x433970BF, including 0x43368A74 tree search
    void hide_scope_panel(void* player); // complete 0x4337F0C1
    // Complete 0x433E273D with the renderer's screen-size query supplied by
    // the frame's viewport. Its two rectangles use the recovered queue writer.
    void draw_scope(ui::RenderQueue&, Point viewport, void* player);
    // Production HUD entry queries the supplied queue after weapon455,
    // preserving the original queue's late virtual18 context association.
    void draw_scope_live(ui::RenderQueue&, void* player);
    void instant_scope(void* player); // 0x433FA01F..0x433FA062, after local/suspended gates

    // Recoil transaction inside calculate_view_hook:433C82F7..433C842A and
    // restoration433C84C3..433C84DD. FOV has its own slot6 view callback.
    SavedRecoilAngles begin_recoil_angles(void* player);
    void restore_recoil_angles(const SavedRecoilAngles&);
    SavedViewHeight begin_view_height(void* player, ViewDuckBindings&); // complete43384DE0
    void restore_view_height(const SavedViewHeight&);
    // Complete433C82DC, with the actual registered secondary interfaces.
    void calculate_view_hook(void* player, void** original_table, ViewDuckBindings&,
                             const engine::OwnerRegistry&, std::uintptr_t first,
                             std::uintptr_t second, std::uintptr_t third);
    // Live player366 entry. Metadata is resolved after observer/duck dispatch,
    // at native433C8480; those real callbacks may replace the current table.
    void calculate_hooked_view(void* player, ViewDuckBindings&, const engine::OwnerRegistry&,
                               std::uintptr_t first, std::uintptr_t second, std::uintptr_t third);

    // Complete 0x433B0541. original_table is the engine's saved vtable;
    // slot 250 receives the three opaque original arguments unchanged.
    void recoil_scale_hook(void* player, void** original_table, std::uintptr_t first,
                           std::uintptr_t second, std::uintptr_t third);
    void recoil_scale_hooked(void* player, const std::uint32_t& encoded_recoil_scale,
                             std::uintptr_t first, std::uintptr_t second, std::uintptr_t third);

private:
    bool scope_visible(void* player);
    template <bool hooked>
    void calculate_view_transaction(void*, void**, ViewDuckBindings&, const engine::OwnerRegistry&,
                                    std::uintptr_t, std::uintptr_t, std::uintptr_t);
    template <bool hooked>
    void recoil_scale_transaction(void*, void**, const std::uint32_t&, std::uintptr_t,
                                  std::uintptr_t, std::uintptr_t);
};
} // namespace skeet::effects
