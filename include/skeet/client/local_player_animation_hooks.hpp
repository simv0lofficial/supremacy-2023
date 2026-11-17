#pragma once
#include "skeet/effects/view.hpp"
#include "skeet/hud/crosshair.hpp"
#include "skeet/misc/network_fix.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/engine/hooks.hpp"

namespace skeet::client {
struct PredictionHistoryOwner;
struct LocalPlayerAnimationHookOffsets {
    std::ptrdiff_t current_command_f58e6118, animation_state_3d52a1a7, strafe_00a5bd68;
    std::ptrdiff_t life_state_9d920e35, observer_mode_2f4e97c4, view_offset_c246315e;
    std::ptrdiff_t model_index_a25d8271, angles_3b1f6ecc, movement_mode_8b5586f6;
    std::ptrdiff_t ducking_a15c3105, bone_accessor_54783141, bone_field_2b554ad0;
    std::ptrdiff_t bone_time_c7e1ecd2, bone_counter_0471e13c, setup_flags_bb78dc43;
};
struct LocalPlayerAnimationHookBindings {
    LocalPlayerAnimationHookOffsets offsets;
    const std::uint8_t& animation_update; //43475BD3
    const std::uintptr_t& strafe_call_site; //434688C4
    PredictionHistoryOwner* const& command_owner; //live434759B4,nonnull gate only
    const std::uint8_t& third_person; //43468A10
    void* const& input; //4346A894
    engine::NativeConVarReference camera_distance; //live43467AA0 xor8985EFCE
    void* const& engine_trace; //43468304
    std::uint8_t* const& trace_flag; //optional434683D0
    effects::ViewRuntime& view;
    effects::ViewDuckBindings& duck;
    const std::uint8_t& command_mode; //43467224
    const std::uint8_t& alternate_command_mode; //4346A740
    const std::int32_t& animation_frame; //*434678F8
    const float& current_time; //*43467EB8
    std::array<float, 3>& view_angles; //4347BD30
    const HotkeyBinding& third_person_key; //43468448/4346845C
    InputBitmap keys;
    lagcomp::SetAbsoluteVector set_absolute_angles; //optional434688C8
    const misc::NetworkFixBindings& network_fix;
    const esp::ScreenOverlayConfig& crosshair_config;
    const hud::CrosshairViewBindings& crosshair;
    void* const& entity_list; //4346A890, crosshair's actual lookup
    const std::uint8_t& suspended; //43468E58
};
class LocalPlayerAnimationHooks {
public:
    explicit LocalPlayerAnimationHooks(const LocalPlayerAnimationHookBindings& bindings) noexcept
        : bindings(bindings) {}
    const LocalPlayerAnimationHookBindings& bindings;
    //222/43341FA7: two original32-bit stack words, forwarded unchanged.
    std::uint32_t update_strafe(void* player, std::uint32_t first, std::uint32_t second,
                                std::uintptr_t caller);
    void update_client_animation(void* player); //224/43329436
    std::uintptr_t network_fix(void* player, void* caller_frame,
                               std::uintptr_t caller); //268/433C6689, preserves original EAX
    void crosshair_view(void* player, void* view_setup, std::uintptr_t caller); //294/433E009A
    // Called after descriptor preparation and before the existing installer.
    // Only these four exact slot/entry pairs are accepted.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
};
} // namespace skeet::client
