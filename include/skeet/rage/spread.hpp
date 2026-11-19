#pragma once
#include "skeet/rage_hitboxes.hpp"
#include <span>

namespace skeet::rage {
struct SpreadRandom {
    void* context;
    // Required genuine vstdlib RandomSeed/RandomFloat bindings.
    void (*seed)(void*, std::int32_t);
    float (*uniform)(void*, float, float);
};
struct SpreadSeed {
    float first_cosine, first_sine, second_cosine, second_sine;
    float first_radius, second_radius;
};
static_assert(sizeof(SpreadSeed) == 0x18);
// Readonly view of a canonical native pointer cell. Indexing resolves its
// current allocation, including initialization and finalizer publication.
class SpreadPlane {
public:
    explicit SpreadPlane(SpreadSeed* const& allocation) noexcept : allocation_(&allocation) {}
    const SpreadSeed* data() const noexcept {
        return *allocation_;
    }
    const SpreadSeed& operator[](std::size_t index) const noexcept {
        return data()[index];
    }
    static constexpr std::size_t size() noexcept {
        return 256;
    }
    const SpreadSeed* begin() const noexcept {
        return data();
    }
    const SpreadSeed* end() const noexcept {
        return data() + size();
    }
    std::span<const SpreadSeed, 256> seeds() const noexcept {
        return std::span<const SpreadSeed, 256>(data(), size());
    }

private:
    SpreadSeed* const* allocation_;
};
struct SpreadTables {
    SpreadPlane standard;
    SpreadPlane shotgun_patterns;
    SpreadTables(SpreadSeed* const& first, SpreadSeed* const& second) noexcept
        : standard(first), shotgun_patterns(second) {}
};
// Numerical loops43402EA8..43402FFD after allocation. Each completed draw or
// trigonometric call writes its native field before the next external call.
// SpreadStorage owns the complete43402E73 allocation/publication sequence.
void fill_spread_plane(std::span<SpreadSeed, 256>, bool shotgun_patterns, const SpreadRandom&,
                       const RageTrigonometry&);

struct SpreadParameters {
    std::uint32_t weapon_id; // weapon context +D8
    std::int32_t fire_mode; // +B0
    float spread; // +D0
    float inaccuracy; // +C4
    float recoil_index; // +100
    std::int32_t pellet_count; // +19C
    bool shotgun_patterns; // resolved ConVar at 43468520
    std::uint8_t weapon_type = 0; // +124; shotgun class is 4
};
struct ShotgunPattern {
    void* context = nullptr;
    // Genuine engine function at 43467DDC. The native code initializes both
    // outputs to zero before calling this binding.
    void (*get)(void*, std::uint32_t, std::int32_t, std::int32_t, float& angle,
                float& radius) = nullptr;
};
// Complete 433C0041, including Revolver secondary and early Negev recoil.
float transform_spread_radius(std::uint32_t weapon_id, std::int32_t fire_mode, float recoil_index,
                              float radius) noexcept;
// Complete 433ADB4F; seed is the native zero-based table index, 0..255.
// Shotgun averaging uses the real engine pattern and RNG dependencies above.
RageSpreadSample generate_spread_sample(const SpreadParameters&, std::uint32_t seed,
                                        const SpreadTables&, const SpreadRandom&,
                                        const ShotgunPattern&, const RageTrigonometry&);

struct SpreadCache {
    std::array<RageSpreadSample, 128> primary;
    std::array<RageSpreadSample, 128> compensated;
    std::uint32_t generation;
};
// One complete engine seed outcome. Pellets remain separate rays.
struct CompleteSpread {
    static constexpr unsigned seed_count = 256, maximum_pellets = 16;
    std::array<std::array<RageSpreadSample, maximum_pellets>, seed_count> rays{};
    unsigned pellets = 0;
    bool valid = false;
};
bool prepare_complete_spread(CompleteSpread&, const SpreadParameters&, const SpreadTables&,
                              const SpreadRandom&, const ShotgunPattern&, const RageTrigonometry&);
// Full preparation loop 433C1977..433C1B9A. Enabled is context flags +178 mask10;
// generation is the weapon marker +AC, cached at aim context +1C4. combined_spread
// is weapon context +160; compensated inaccuracy is combined_spread - spread.
bool prepare_spread_cache(SpreadCache&, const SpreadParameters&, float combined_spread,
                          std::uint32_t generation, bool enabled, const SpreadTables&,
                          const SpreadRandom&, const ShotgunPattern&, const RageTrigonometry&);
// The 32-sample consumer at 433600FE advances 0x20 bytes through the 8-byte cache.
std::array<RageSpreadSample, 32> hitchance_spread_samples(const SpreadCache&) noexcept;
std::uint32_t cached_spread_seed(std::uint32_t index) noexcept;
} // namespace skeet::rage
