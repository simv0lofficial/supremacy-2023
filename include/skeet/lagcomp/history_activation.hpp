#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::legit {
class LifecycleOwner;
}
namespace skeet::lagcomp {
struct HistoryActivationBindings {
    const std::uint8_t* prediction_suppressed; // address of global434693EC
    // Address of global4346A3A4, whose optional value points to the live local
    // player cell. Both levels are read by433C0714 after its flag gate.
    void* const* const* local_player;
    std::ptrdiff_t player_prediction_disabled; // decodedBF567619 byte offset
    legit::LifecycleOwner* const* configuration; // canonical owner slot4347588C
    const std::uint8_t* recording_enabled; // address of global43467224
};

// Complete433C0714 boolean result. The caller publishes it to context+34
// before asking the engine's82/93 gates about recording availability.
bool history_prediction_available(const HistoryActivationBindings&) noexcept;

// Complete43401219. Resolve the optional actual LEGIT owner, then read its
// canonical nine enabled/trigger-enabled/trigger-reaction/accuracy entries.
// Source owners are never interpreted with the native allocation's offsets.
bool history_configuration_active(const HistoryActivationBindings&) noexcept;

} // namespace skeet::lagcomp
