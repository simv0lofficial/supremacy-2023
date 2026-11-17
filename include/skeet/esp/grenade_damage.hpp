#pragma once
#include "grenade_prediction.hpp"
#include "skeet/rage_hitboxes.hpp"
#include <array>

namespace skeet::engine {
class DecodedOffset;
}
namespace skeet::esp {
struct GrenadeDamagePlayer {
    std::uint32_t entity;
    Vector3 origin; // decoded A9EE6062, used by the five body samples
    Vector3 distance_origin; // caller-selected live/history/extrapolated origin
    Vector3 absolute_angles; // entity virtual +2C
    float duck_amount; // decoded 3309C0CF
    std::int32_t armor;
    bool alternate_eye; // decoded D070684C
    Vector3 virtual_origin; // entity virtual +28, alternate-eye path only
    Vector3 view_offset; // decoded C246315E, alternate-eye path only
};
enum class GrenadeDamageFilterKind {
    ignore_entity,
    exclude_players
};
struct GrenadeDamageFilter {
    GrenadeDamageFilterKind kind;
    std::uint32_t ignored_entity;
};
struct GrenadeDamageFilterEntity {
    bool base_filter_accepts; // actual engine filter at *434758B4
    bool converted_entity_present; // engine 4333D32B conversion result
    bool virtual_278;
    bool virtual_298;
};
// Complete client extension 4334EA3E to the genuine engine filter. The ordinary
// filter and GetTraceType stay actual engine dependencies (433E1CF5/433F9028).
bool grenade_damage_should_hit(GrenadeDamageFilterKind, const GrenadeDamageFilterEntity&) noexcept;
struct GrenadeDamageTrace {
    Vector3 end;
    float fraction = 1;
    bool all_solid = false, start_solid = false;
    std::uint32_t entity = 0;
};
class GrenadeDamageEngine {
public:
    virtual ~GrenadeDamageEngine() = default;
    virtual GrenadeDamageTrace trace(const GrenadeRay&, std::uint32_t mask,
                                     const GrenadeDamageFilter&) = 0;
    virtual std::uint32_t world_entity() = 0; // entity-list +0C, index zero
    virtual std::uint32_t owner_handle(std::uint32_t entity) = 0; // decoded 8D1BD8D0
    virtual std::uint32_t entity_for_handle(std::uint32_t) = 0; // entity-list +10
    virtual std::uint32_t physics_object(std::uint32_t entity) = 0; // A249061A minus C
    virtual std::int32_t physics_material(std::uint32_t object) = 0; // virtual +98
    virtual float surface_density(std::int32_t material) = 0; // surface-props +10
};
struct GrenadeDamageMath {
    RageTrigonometry trigonometry; // actual imported sinf/cosf
    float (*exponential)(float); // actual import 7750AAF0, __libm_sse2_expf
};
struct GrenadeDamageResult {
    float damage = 0, exposure = 0;
    Vector3 sample_position{};
    Vector3 distance_point{};
    bool in_range = false;
};
// Complete 4331E7F5; also inlined in the eye-position helper 433849B5.
float grenade_player_view_height(float duck_amount) noexcept;
// The repeated native two-ray/material path, e.g. 433BB853..433BBA9F.
float grenade_surface_transmission(Vector3 explosion, Vector3 sample, std::uint32_t target,
                                   GrenadeDamageEngine&);
// Shared geometry core for the live owner. Eye capture precedes the absolute
// angle getter; body origin is read after that getter, as in the native chain.
std::array<Vector3, 5> make_grenade_damage_samples(Vector3 body_origin, Vector3 captured_eye,
                                                   float captured_eye_offset, Vector3 captured_top,
                                                   Vector3 absolute_angles,
                                                   const GrenadeDamageMath&);
// Raw Gaussian result. The live caller performs its later armor-property read.
GrenadeDamageResult evaluate_he_damage_geometry(Vector3 explosion, std::uint32_t target,
                                                Vector3 distance_point,
                                                const std::array<Vector3, 5>&, GrenadeDamageEngine&,
                                                const GrenadeDamageMath&);
// The third body ray decodes and reads the real origin property after the
// preceding two engine traces, including every offset candidate.
GrenadeDamageResult
evaluate_he_damage_live_geometry(Vector3 explosion, std::uint32_t target, Vector3 distance_point,
                                 const std::array<Vector3, 5>&, const void* actual_entity,
                                 const engine::DecodedOffset& origin_offset, GrenadeDamageEngine&,
                                 const GrenadeDamageMath&);
// Per-target native chain 433BB35C..433BE1E3, duplicated in 43337BC0.
// The caller owns player eligibility and the selected distance_origin. All five
// body rays, thirteen collision-adjusted offset probes, material transmission,
// strict exposure selection and the final Gaussian/armor arithmetic stay here.
GrenadeDamageResult evaluate_he_damage(Vector3 explosion, const GrenadeDamagePlayer&,
                                       GrenadeDamageEngine&, const GrenadeDamageMath&);
} // namespace skeet::esp
