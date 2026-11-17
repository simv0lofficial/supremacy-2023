#pragma once
#include "viewmodel_lag.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include <span>

namespace skeet::effects {
class ViewRuntime;
}
namespace skeet::ui {
struct SettingsMenuValues;
}

namespace skeet::misc {
class ViewmodelLagOwner;
inline constexpr std::uint32_t viewmodel_lag_owner_slot = 0x43475860;
struct ViewmodelHookInputs {
    void* const& engine;
    void* const& entity_list;
    void* const& input;
    void* const& local_player;
    engine::ConVarBinding interpolation_cvar;
    float& prediction_time;
    const float& interval;
    const float& interpolation_fraction;
    ViewmodelLagOffsets offsets;
    std::ptrdiff_t player_tick_base;
    const std::uint32_t& encoded_recoil_scale; //raw global4346A534
};
struct ViewmodelHookReplacement {
    std::uint32_t slot, native_entry;
    void* source_entry;
};
// Exact captured descriptor recipe14, contexts1994..2011. The protected
// scheduler decides when it runs; this family introduces no startup ordering.
inline constexpr std::uint32_t viewmodel_hook_descriptor = 0x43475B34;
inline constexpr std::uint32_t viewmodel_hook_source_global = 0x4346A55C;
inline constexpr std::uint32_t viewmodel_hook_class_hash = 0x17CEECD0;
inline constexpr char viewmodel_hook_class_name[] = ".?AVC_PredictedViewModel@@";
inline constexpr std::uint32_t viewmodel_hook_replacements_operand = 0x448;
std::span<const ViewmodelHookReplacement, 6> viewmodel_hook_replacements();

class ViewmodelHooksRuntime {
public:
    ViewmodelHooksRuntime(ViewmodelLagState& owner_state, ViewmodelLagOwner*& shared_owner,
                          const ui::SettingsMenuValues&, ViewmodelHookInputs, effects::ViewRuntime&,
                          engine::VtableHookRegistry&, engine::HookDescriptorStorage&);
    ViewmodelHooksRuntime(const ViewmodelHooksRuntime&) = delete;
    ViewmodelHooksRuntime& operator=(const ViewmodelHooksRuntime&) = delete;
    // Every feature binding reads the same Settings global byte and
    // live engine slots. The sequence history remains owned by SequenceRuntime.
    const ViewmodelLagBindings& feature_bindings() const noexcept;
    ViewmodelLagState& owner_state() const noexcept {
        return owner_state_;
    }
    ViewmodelLagOwner*& shared_owner() const noexcept {
        return owner_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return image_;
    }
    // Recipe14's concrete producer path: real executable-prefix clone, then
    // these six replacements. The genuine module/pattern resolver supplies
    // original_table; the later actual-receiver installation remains separate.
    void prepare(void** original_table, engine::JobMemory, engine::HookDescriptorSystem);
    void prepare_from_module(std::byte* client_module, engine::NativeImageEnvironment,
                             engine::JobMemory, engine::HookDescriptorSystem);
    // Called by the captured descriptor producer after cloning the genuine
    // source table, before that scheduler installs it on an actual receiver.
    void bind_replacements() noexcept;
    engine::VtableHookNode* install(void* receiver);
    std::uintptr_t destroy(void* receiver, std::uint32_t argument);
    bool method113(void* receiver, float);
    std::uint32_t method206(void* receiver, std::uint32_t, std::uint32_t, std::uint32_t,
                            std::uint32_t, float);
    void method247(void* receiver, std::int32_t);
    std::uint32_t method248(void* receiver, std::uint32_t, std::uint32_t);
    void method250(void* receiver, std::uintptr_t, std::uintptr_t, std::uintptr_t);
    void update_entities(void* player, std::uintptr_t incoming);
    void frame_update();

private:
    ViewmodelLagState& owner_state_;
    ViewmodelLagOwner*& owner_; //canonical full owner pointer43475860
    const std::uint8_t& enabled_;
    ViewmodelHookInputs inputs_;
    effects::ViewRuntime& view_;
    engine::VtableHookRegistry& registry_;
    const engine::NativeDataImage& image_;
    engine::VtableHookMetadata& descriptor_; //same retained-image43475B34 cell
    ViewmodelLagBindings feature_;
    ViewmodelLagHookBindings hook_bindings(const ViewmodelLagBindings&) const;
};

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_VIEWMODEL_HOOK_ABI __fastcall
#else
#define SKEET_VIEWMODEL_HOOK_ABI
#endif
std::uintptr_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_destroy_hook(void*, void*, std::uint32_t);
bool SKEET_VIEWMODEL_HOOK_ABI viewmodel_113_hook(void*, void*, float);
std::uint32_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_206_hook(void*, void*, std::uint32_t,
                                                          std::uint32_t, std::uint32_t,
                                                          std::uint32_t, float);
void SKEET_VIEWMODEL_HOOK_ABI viewmodel_247_hook(void*, void*, std::int32_t);
std::uint32_t SKEET_VIEWMODEL_HOOK_ABI viewmodel_248_hook(void*, void*, std::uint32_t,
                                                          std::uint32_t);
void SKEET_VIEWMODEL_HOOK_ABI viewmodel_250_hook(void*, void*, std::uintptr_t, std::uintptr_t,
                                                 std::uintptr_t);
#undef SKEET_VIEWMODEL_HOOK_ABI
} //namespace skeet::misc
