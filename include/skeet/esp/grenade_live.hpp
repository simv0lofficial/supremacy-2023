#pragma once
#include "grenade_auxiliary.hpp"
#include "grenade_warning_owner.hpp"
#include <memory>

namespace skeet::misc {
struct GrenadeCommandState;
struct GrenadePredictionConfig;
}
namespace skeet::esp {
struct GrenadeEnvironmentBindings;
struct GrenadeLiveBindings;

// Production scene reader. Every method implements its native stage directly
// from real engine interfaces, decoded offsets and canonical runtime owners.
// No feature callbacks or precomputed per-player candidate arrays are accepted.
class GrenadeLiveScene {
public:
    explicit GrenadeLiveScene(const GrenadeLiveBindings&);
    ~GrenadeLiveScene();
    GrenadeLiveScene(const GrenadeLiveScene&) = delete;
    GrenadeLiveScene& operator=(const GrenadeLiveScene&) = delete;
    GrenadeTraceProvider trace() noexcept;
    const GrenadeEnvironmentBindings& environment() const noexcept;
    GrenadeWarningEngine& engine() noexcept;
    MovementCollisionEngine& movement() noexcept;
    const GrenadeDamageMath& damage_math() const noexcept;
    const GrenadeAuxiliaryLiveContext& auxiliary();
    // Candidate capture occurs at each actual client ordinal, after preceding
    // targets' traces. The native selector is not fed a bulk snapshot.
    GrenadeDamageSelection select_damage(Vector3 explosion, const IntegerConfig& threshold);
    // Re-reads the stage's scalar/local fields at their actual use sites.
    void assess_warning(GrenadeWarningRecord&, void* selected_player,
                        std::uint32_t& command_buttons);
    // incoming is the actual +44 stride8 / +4044 mutable packet container.
    void update_remote_warnings(GrenadeWarningOwner&, const GrenadeEspConfig&,
                                void* selected_player, std::uintptr_t incoming);
    void draw_prediction(ui::RenderQueue&, const misc::GrenadePredictionConfig&,
                         const GrenadeEspConfig&, GrenadeTrajectoryOwner&,
                         misc::GrenadeCommandState&, GrenadeWarningOwner&);

private:
    struct Storage;
    std::unique_ptr<Storage> storage_;
};
} // namespace skeet::esp
