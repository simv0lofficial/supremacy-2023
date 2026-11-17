#pragma once
#include "sound_overlay.hpp"
#include "effects_config.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::esp {

struct PacketTracerOffsets {
    DormantRadarOffsets radar;
    std::ptrdiff_t recoil_index; // 6777C9CE, float
    std::ptrdiff_t seed; // F300F6B0, int32; valid native table index0..255
    std::ptrdiff_t fire_mode; // EC1739AA, int32
    std::ptrdiff_t weapon_id; // 571B7D9A, uint16
    std::ptrdiff_t inaccuracy; // C6C62EBA, float
    std::ptrdiff_t spread; // E9392423, float
    std::ptrdiff_t angles; // F62F1430, Vector3; native roll is replaced by0
};
struct PacketTracerState {
    void* beams = nullptr; // owner+60, original4346A898 interface
    std::uint32_t first = 0, count = 0; // owner+64/+68
    std::array<void*, 3> recent{}; // owner+6C..74, same live beam pointers
};
struct PacketTracerBindings {
    PacketTracerOffsets offsets;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& trigonometry;
    engine::NativeConVarReference shotgun_patterns_convar; //43468520^394BFF56
    void* const& weapon_system; // optional actual43467B54
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; // genuine external43475B90
    const anti_aim::TargetTraceEntities& trace_entities;
    const float* const& prediction_time; // *43467EB8, reloaded after beam creation
};

// Engine-owned BeamInfo_t used by actual IViewRenderBeams::CreateBeamPoints.
// The source preserves the native zeroes as well as explicitly assigned fields.
struct PacketBeamDescription {
    std::int32_t type = 0;
    void* start_entity = nullptr;
    std::int32_t start_attachment = 0;
    void* end_entity = nullptr;
    std::int32_t end_attachment = 0;
    Vector3 start{}, end{};
    std::int32_t model_index = 0;
    const char* model_name = nullptr;
    std::int32_t halo_index = -1;
    const char* halo_name = nullptr;
    float halo_scale = 0, life = 0, width = 3, end_width = 3;
    float fade_length = 0, amplitude = 0, brightness = 0, speed = 0;
    std::int32_t start_frame = 0;
    float frame_rate = 0, red = 0, green = 0, blue = 0;
    std::uint8_t renderable = 1;
    std::array<std::byte, 3> padding_71{};
    std::int32_t segments = -1, flags = 8;
    Vector3 center{};
    float start_radius = 0, end_radius = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(PacketBeamDescription) == 0x90);
static_assert(offsetof(PacketBeamDescription, start) == 0x14);
static_assert(offsetof(PacketBeamDescription, model_index) == 0x2c);
static_assert(offsetof(PacketBeamDescription, brightness) == 0x54);
static_assert(offsetof(PacketBeamDescription, flags) == 0x78);
#endif

// Complete433FBE76..433FC6BF. The same decoded packet first updates dormant
// radar geometry, then reconstructs the enemy shot and creates its beam.
void process_esp_packet(const void* packet, PacketTracerState&, SoundOverlay&,
                        const SoundOverlayContext&, const EffectsSettings&,
                        const PacketTracerBindings&);

#if defined(_M_IX86) || defined(__i386__)
using EspPacketEntry = void(__fastcall*)(void*, const void*);
#else
using EspPacketEntry = void (*)(void*, const void*);
#endif
struct EspPacketEndpoint {
    const EspPacketEntry* vtable = nullptr;
};
// Actual owner+20 interface4347082C, registered by43400957 in4347584C.
class PacketTracerHandler final : public EspPacketEndpoint {
public:
    PacketTracerHandler(engine::OwnerRegistryBinding, PacketTracerState&, SoundOverlay&,
                        const SoundOverlayContext&, const EffectsSettings&,
                        const PacketTracerBindings&);

private:
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall update(void*, const void*);
#else
    static void update(void*, const void*);
#endif
    inline static constexpr EspPacketEntry entries_[1]{update};
    PacketTracerState& state_;
    SoundOverlay& sounds_;
    const SoundOverlayContext& sound_context_;
    const EffectsSettings& settings_;
    const PacketTracerBindings& bindings_;
};

} // namespace skeet::esp
