#pragma once
#include "command_frame.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/prediction/command.hpp"
#include "skeet/weapon/fire_state.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::client {
class CommandCopyOverrideScope;
struct CommandEyeAdjustmentContext;
struct CommandEyeAdjustmentSourceContext;
struct CommandRecaptureOffsets {
    lagcomp::EntityNetvars common;
    std::ptrdiff_t active_weapon; // CD812654
    std::ptrdiff_t byte_7501cb91;
    std::ptrdiff_t view_offset; // C246315E
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t field_66146802;
    std::ptrdiff_t field_5ae32976;
    std::ptrdiff_t weapon_field_572aa59f;
};
struct CommandRecaptureContext {
    void* player;
    void* entity_list;
    CommandRecaptureOffsets offsets;
    const weapon::FireStateBindings& fire;
    engine::NativeConVarReference recoil_scale_convar; //4346794C^C429E1BF; terminal+2C
    const std::uint8_t* secondary_override; // actual434693D4
    const RageTrigonometry& trigonometry;
    std::array<float, 3>& published_origin; //43475BD4
};
// Complete ordinary-player433AE483. Reads the terminal ConVar on every call.
std::array<float, 3> capture_command_recoil(void* player, std::ptrdiff_t recoil_offset,
                                            void* recoil_scale_convar);
//433D4558..433D46F4, before the second prediction transaction is restored.
void update_predicted_command_weapon(const CommandRecaptureContext&);
//433D4709..433D4DBE, after the actual CommandPrediction::restore().
void capture_predicted_command_frame(CommandFrame&, Command&, const CommandRecaptureContext&);
// Shared physical state capture, excluding the recapture-only flags|=1.
// Initial433D3Axx..405x and second prediction retain their own surrounding gates.
enum class CommandCapturePass {
    initial,
    predicted
};
void capture_command_physical_frame(CommandFrame&, Command&, const CommandRecaptureContext&,
                                    CommandCapturePass);
// Complete second-prediction tail433D4558..433D4DEA. The active prediction
// and outer override were constructed by the master at4404 and439D.
void recapture_command_prediction(CommandFrame&, Command&, prediction::CommandPrediction&,
                                  CommandCopyOverrideScope&, const CommandRecaptureContext&,
                                  const CommandEyeAdjustmentContext&);
void recapture_command_prediction(CommandFrame&, Command&, prediction::CommandPrediction&,
                                  CommandCopyOverrideScope&, const CommandRecaptureContext&,
                                  const CommandEyeAdjustmentSourceContext&);
} // namespace skeet::client
