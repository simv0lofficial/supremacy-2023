#pragma once
#include "skeet/lagcomp/prediction_scope.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <cstdint>

namespace skeet::prediction {

#if defined(_M_IX86) || defined(__i386__)
using SeedGenerator = std::uint32_t(__fastcall*)(std::int32_t);
#else
using SeedGenerator = std::uint32_t (*)(std::int32_t);
#endif
// 433D3457..433D3470: *434679AC receives command +4 in ECX.
// Its engine result is masked to 31 bits and written to command +40.
void update_command_seed(client::Command&, SeedGenerator);

// The native list behind 43467C58 uses next +4 and enabled +8 on x86.
struct PredictionFlagNode {
    PredictionFlagNode* previous;
    PredictionFlagNode* next;
    std::uint8_t enabled;
};
void set_prediction_flags(PredictionFlagNode* first, std::uint8_t value) noexcept;

struct TimeTargets {
    float* frame_time; // 43467B04 +14
    float* current_time; // *43467EB8
    std::uint8_t* in_prediction; // *43469264 +8
};

// Inlined at 433D33CF..433D345A (also 433D321A and 433D57B0).
// The 433B0A4F destructor restores flag, current time, then frame time.
class TimeScope {
public:
    TimeScope(TimeTargets targets, bool paused, float interval, std::int32_t tick_base) noexcept;
    ~TimeScope() {
        restore();
    }
    TimeScope(const TimeScope&) = delete;
    TimeScope& operator=(const TimeScope&) = delete;
    void restore() noexcept;

private:
    TimeTargets targets;
    float frame_time;
    float current_time;
    std::uint8_t in_prediction;
};

struct CommandPredictionConVars {
    engine::NativeConVarReference aaa77b8a;
    engine::NativeConVarReference de58ce34;
};
struct CommandBindings {
    //The native owner reloads these pointer cells between engine methods.
    void* const& prediction; // *43469264: SetupMove +50, FinishMove +54
    void* const& move_helper; // *43468EC8: SetHost +4
    void* const& game_movement; // *4346A7F8: ProcessMovement +4
    void* move_data; // 43478CD8
    PredictionFlagNode** flag_head; // *43467C58, read again on destruction
    lagcomp::PredictionTargets targets;
    // Production captures these two live cells at each433697EE entry. The
    // resulting PredictionScope retains those per-call objects for restore.
    const CommandPredictionConVars* live_convars = nullptr;
};
struct CommandPlayerOffsets {
    std::ptrdiff_t current_command_f58e6118;
    std::ptrdiff_t vector_6610746e;
};
//Production433697EE targets depend on this call's copied command/player.
//The template supplies resolved global/ConVar addresses, never these local
//targets. This also binds prediction+8/+18 and GameMovement+8 directly.
CommandBindings bind_command_prediction(const CommandBindings&, void* const& prediction,
                                        void* player, client::Command& copied_command,
                                        const CommandPlayerOffsets&) noexcept;

// 433D4404..433D4558 runs this sequence with a copied command at stack +340.
// Engine methods are actual vtable boundaries; the return-spoof trampoline
// 433E533F changes their return path, not their arguments or side effects.
// The owner continues weapon handling before 433D46F4 ends these scopes.
class CommandPrediction {
public:
    CommandPrediction(const CommandBindings&, void* player, void* command,
                      std::uint32_t random_seed);
    ~CommandPrediction() {
        restore();
    }
    CommandPrediction(const CommandPrediction&) = delete;
    CommandPrediction& operator=(const CommandPrediction&) = delete;
    void restore() noexcept;

private:
    const CommandBindings& bindings;
    lagcomp::PredictionScope saved;
    bool active = true;
};
} // namespace skeet::prediction
