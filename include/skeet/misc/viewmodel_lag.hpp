#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/skins/sequences.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::misc {
struct ViewmodelCommandTick {
    std::int32_t command, tick_base;
};
struct ViewmodelLagState {
    // Bind the native snapshot region to the one canonical Skins history.
    // The constructor deliberately leaves native unassigned words untouched;
    // initialize_viewmodel_lag performs43486C3D's sparse writes later.
    explicit ViewmodelLagState(std::array<skins::SequenceHistory, 150>& history) noexcept
        : snapshots(history) {}
    // Shared native owner43475860. The receiver biases are +8 for prediction,
    // +10 for frame processing, +18 for command phases and +20 for networking.
    std::int32_t tick_base, last_command, prediction_generation; // +2C/+30/+34
    std::int32_t command_number, previous_query, previous_generation; // +38/+3C/+40
    float previous_animation_time; // +44
    std::array<ViewmodelCommandTick, 150> ticks; // +48
    std::array<skins::SequenceHistory, 150>& snapshots; // +4F8, canonical skins history
};
struct ViewmodelLagOffsets {
    std::ptrdiff_t handles; // player71DCEE42, exactly two handles
    std::ptrdiff_t flags; // viewmodel38CA5228, bit20 rejects the entity
    std::ptrdiff_t animation_base; // D5FD2E2C; current/old time at -8/-4
    std::ptrdiff_t weapon_handle; // A116369A
    std::ptrdiff_t sequence; // A25D8271; cached float at +30
    std::ptrdiff_t animation_token_08; // 695FF325
    std::ptrdiff_t animation_token_0c; // AEF72A04
    std::ptrdiff_t field_18; // 43D8CA19
    std::ptrdiff_t player_byte; // 53E7C7D2
    std::ptrdiff_t selected_model; // 47877DEB, int16
    std::ptrdiff_t animation_value; // BEFF3172, float
};
struct ViewmodelLagBindings {
    const std::uint8_t& enabled; // actual Settings global4346A328
    void* const& engine;
    void* const& entity_list;
    const float* current_time; // *43467EB8
    const float* interval; // *4346A338
    engine::ConVarBinding interpolation_cvar; // live4346A850^478720C2
    ViewmodelLagOffsets offsets;
};
// Native43486C3D sparse state initialization and43341C91 lifecycle reset.
void initialize_viewmodel_lag(ViewmodelLagState&) noexcept;
void reset_viewmodel_lag(ViewmodelLagState&) noexcept;
// Complete433FA91C and433AFBCC: both phase0/phase1 bodies publish the same
// command/tick pair without checking the Settings byte.
void record_viewmodel_command(ViewmodelLagState&, const client::Command&) noexcept;
// Complete43325312 /433D0ABD, called through the network owner at native+20.
void record_viewmodel_command_range(ViewmodelLagState&, const ViewmodelLagBindings&,
                                    std::int32_t final_command, std::int32_t count);
void advance_viewmodel_prediction(ViewmodelLagState&, const ViewmodelLagBindings&);
void consume_viewmodel_prediction(ViewmodelLagState&) noexcept; // complete4337D213
// Complete prediction interface4346F0D4 slots0/1/3.
void prepare_viewmodel_prediction(ViewmodelLagState&, const ViewmodelLagBindings&, void* player,
                                  const client::Command&);
void capture_viewmodel_prediction(ViewmodelLagState&, const ViewmodelLagBindings&, void* player,
                                  const client::Command&);
void copy_previous_viewmodel_snapshot(ViewmodelLagState&, const ViewmodelLagBindings&,
                                      std::int32_t command_number);
// Complete433833B0 and concrete43407DE2. Keeps both demo/HLTV check pairs and
// both acknowledged-command calls; they are separate observable engine reads.
void repair_viewmodel_lag(ViewmodelLagState&, const ViewmodelLagBindings&, void* player);

struct ViewmodelLagHookBindings {
    const ViewmodelLagBindings& runtime;
    void* input; // 4346A894, 150 client::Command entries at +F0
    void* local_player; // 43475808; may be null
    std::ptrdiff_t player_tick_base; // 434675C8 /3EDECE61
    float* prediction_time; // mutable *43467EB8
    const float* interpolation_fraction; // 43467B04+24
};
// Complete native hook bodies. They call the genuine original engine methods
// from receiver.table[-2]'s metadata, at the original call point. Early-return
// gates do not read that descriptor. Four integer arguments to
// method206 retain the opaque engine argument words; its fifth is a float.
bool viewmodel_method_113(ViewmodelLagState*, const ViewmodelLagHookBindings&, void* receiver,
                          float);
std::uint32_t viewmodel_method_206(ViewmodelLagState*, const ViewmodelLagHookBindings&,
                                   void* receiver, std::array<std::uint32_t, 4> arguments, float);
void viewmodel_method_247(ViewmodelLagState*, const ViewmodelLagHookBindings&, void* receiver,
                          std::int32_t argument);
} //namespace skeet::misc
