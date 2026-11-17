#pragma once
#include "records.hpp"

namespace skeet::lagcomp {
struct PredictionTargets {
    std::uint8_t* in_prediction;
    std::uint8_t* first_time_predicted;
    std::uint8_t* command_predicted;
    std::uint32_t* random_seed;
    std::uint32_t* prediction_player;
    std::uint32_t* player_current_command;
    void* convar_aaa77b8a;
    void* convar_de58ce34;
    std::uint32_t* field_a7f8_08;
    Vector* player_field_6610746e;
};
// ConVar parent is at +0x1C. Integer encodings at +0x2C and +0x3C XOR the
// terminal object's address; the two copies must both be restored.
std::uint32_t read_prediction_convar(void* object) noexcept;
void write_prediction_convar(void* object, std::uint32_t bits) noexcept;

// Complete valid-player path of 433697EE and complete 433E2143. The target
// bindings are the engine globals/netvars resolved by their owning runtime.
// restore() is idempotent and clears every restored target, in native order.
class PredictionScope {
public:
    PredictionScope(const PredictionTargets&, std::uint32_t player_address,
                    std::uint32_t command_address, std::uint32_t seed) noexcept;
    ~PredictionScope() {
        restore();
    }
    PredictionScope(const PredictionScope&) = delete;
    PredictionScope& operator=(const PredictionScope&) = delete;
    void restore() noexcept;

private:
    PredictionTargets targets;
    std::array<std::uint8_t, 3> bytes;
    std::array<std::uint32_t, 6> words;
    Vector vector;
};
} // namespace skeet::lagcomp
