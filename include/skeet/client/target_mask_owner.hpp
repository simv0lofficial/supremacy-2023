#pragma once
#include "command_frame.hpp"
#include "skeet/config.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <array>
#include <cstdint>

namespace skeet::client {

inline constexpr std::uint32_t target_mask_feature_hash = 0x347A4FA6u;

// Canonical static descriptor43467974, value43467984. CRT43490C68 appends
// this field without changing metadata or its stored value. Keep one settings
// object alive through startup and every TargetMaskOwner that borrows it.
struct TargetMaskSettings {
    BooleanConfig enabled{{{nullptr, nullptr, target_mask_feature_hash}, 1, ConfigType::boolean, 0},
                          0};
};

struct TargetMaskRecord {
    std::int32_t receive_tick = 0;
    std::array<std::uint32_t, 2> targets{};
};
static_assert(sizeof(TargetMaskRecord) == 12);

// One canonical owner shared by command table43470478, message observer
// table4346F334, entity-update table4346F98C and reset table43470114.
// Offsets below refer to its command interface, which is full-owner+8.
struct TargetMaskState {
    std::int32_t accepted_tick = 0; // +1C
    std::uint8_t has_accepted_tick = 0; // +20
    std::uint8_t attack_pending = 0; // +24
    std::uint32_t first = 0, count = 0; // +2C/+30
    std::array<TargetMaskRecord, 128> records{}; // +34
};

struct TargetMaskBindings {
    void* engine_client; // 4346A514, slots14/82/93
    const float* receive_interval; // [43467B04]+20
    const std::uint8_t* rage_enabled; // 43467224
    engine::ConVarBinding lag_compensation; //live [43467A78] XOR3AE06513
    const double* real_latency_outgoing; // 4347A500
    const double* real_latency_incoming; // 4347B118
};

class TargetMaskOwner {
public:
    // Startup bindings exist before CRT43490C68 appends this descriptor.
    // Borrow its canonical value directly without registering or resetting it.
    TargetMaskOwner(TargetMaskSettings&, TargetMaskBindings) noexcept;
    // The native feature is Boolean descriptor43467974, value43467984.
    // Bind its registered hash/type rather than manufacturing another switch.
    TargetMaskOwner(ConfigRegistries&, TargetMaskBindings);
    TargetMaskState state;
    const std::uint8_t& enabled;
    TargetMaskBindings bindings;
};

// Complete owner reset4332F1D9. Recorded payloads and accepted_tick survive;
// their head/count and the two validity/attack bytes are cleared.
void reset_target_mask_owner(TargetMaskOwner&) noexcept;

// Complete4336AC2E, entity-update interface slot0. Incoming status uses its
// actual flags+24 and byte+10; this reset preserves the attack bypass byte.
void observe_target_mask_update_status(TargetMaskOwner&, const void* native_status) noexcept;

// Complete4336526D, entity-update interface slot1. The unchanged incoming
// stack argument points to an object whose +8 is the visibility payload.
// Entity indices1..64 map to bit(index&63), including64 -> bit0.
void capture_target_mask_update(TargetMaskOwner&, std::uintptr_t incoming);

// Complete4332CC6A, observer interface slot0. The real message's virtual7
// identifies type4; its tick+0C is updated in place and retained in this owner.
void process_target_mask_message(TargetMaskOwner&, void* native_message);
// Native observer EDX holds a context whose +4 is that message pointer.
void process_target_mask_message_context(TargetMaskOwner&, const void* native_context);

// Command table43470478 phases4/7, complete4340029E and43386E3E.
void build_command_target_mask(TargetMaskOwner&, CommandFrame&) noexcept;
void mark_target_mask_attack(TargetMaskOwner&, const CommandFrame&, const Command&) noexcept;

} // namespace skeet::client
