#pragma once
#include "command_phase_registry.hpp"
#include "command_frame.hpp"
#include "command_empty_tick_receiver.hpp"
#include "skeet/legit/command.hpp"
#include "skeet/legit/command_source.hpp"
#include "skeet/hud/penetration_runtime.hpp"
#include "skeet/hud/screen_overlay_update.hpp"
#include "rejected_player_update.hpp"

namespace skeet::client {
struct LegitCommandOwner {
private:
    struct Standalone {
        legit::RuntimeBindings& bindings;
        void* engine;
    };
    std::variant<Standalone, legit::CommandSource*> source_;

public:
    legit::RuntimeState& state;
    const legit::RuntimeSettings& settings;
    NativeCommandEmptyTickReceiver empty_tick_receiver{};
    LegitCommandOwner(legit::RuntimeState& s, const legit::RuntimeSettings& configuration,
                      legit::RuntimeBindings& bindings, void* engine) noexcept
        : source_(Standalone{bindings, engine}), state(s), settings(configuration) {}
    LegitCommandOwner(legit::RuntimeState& s, const legit::RuntimeSettings& configuration,
                      legit::CommandSource& source) noexcept
        : source_(&source), state(s), settings(configuration) {}
    void capture_view();
    void clear_allocated_storage();
    void process(CommandFrame&, void* player, Command&);
    void reset();
    void pending(Command&, float);
};
struct PenetrationReticleCommandOwner {
    hud::PenetrationReticleState& state;
    const esp::ScreenOverlayConfig& settings;
    const hud::PenetrationReticleEnvironment& bindings;
};
struct ScreenOverlayCommandOwner {
    hud::ScreenOverlayState& state;
    hud::ScreenOverlayPublisher& publisher;
    const esp::ScreenOverlayConfig& settings;
    const hud::ScreenOverlayFields& offsets;
    const hud::ScreenOverlayUpdateBindings& bindings;
};
struct SharedEspCommandOwner {
    const RejectedPlayerUpdateContext& bindings;
};
//A missing custom source body is rejected before a command mutates engine
//state. Return-only native slots are recognized from the exact phase manifest.
bool command_phase_source_bound(CommandOwnerId, std::uint32_t phase);
const NativeCommandPhaseVtable& source_command_phase_table(CommandOwnerId);
bool is_source_command_phase_table(const NativeCommandPhaseVtable*) noexcept;
} //namespace skeet::client
