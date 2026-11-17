#pragma once
#include "skeet/esp/effects_config.hpp"
#include <array>
#include <cstdint>

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::effects {

// 0x4334ED58. Bits select wall alpha, fullbright's $phong check,
// translucent fullbright, and night color, respectively.
std::uint8_t classify_world_material(void* material, std::uint32_t texture_group_hash, bool dust2);

struct WorldMaterialChange {
    bool dust2 = false;
    bool refresh = false;
    bool alpha = false;
    float wall_alpha = 1.0f;
    bool fullbright = false;
    bool night = false;
    std::array<float, 3> color{};
};
// Complete 0x433D63B7, including excluded groups and the nine owned materials.
void apply_world_material(void* material, const WorldMaterialChange& change,
                          const std::array<void*, 9>& excluded_materials);

struct WorldInterfaces {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_material_ = nullptr;
    void* standalone_static_props_ = nullptr;
    void* standalone_entities_ = nullptr;
    const std::int32_t* standalone_maximum_ = nullptr;
    void* standalone_class_ = nullptr;
    void* standalone_smoke_ = nullptr;

public:
    WorldInterfaces() noexcept
        : engine(standalone_engine_), material_system(standalone_material_),
          static_props(standalone_static_props_), entity_list(standalone_entities_),
          max_clients(standalone_maximum_), tonemap_client_class(standalone_class_),
          smoke_registry(standalone_smoke_) {}
    explicit WorldInterfaces(engine::NativeDataImage&);
    WorldInterfaces(const WorldInterfaces&) = delete;
    WorldInterfaces& operator=(const WorldInterfaces&) = delete;
    void*& engine; //live4346A514
    void*& material_system; //live4346A4E0
    void*& static_props; //live43467014
    void*& entity_list; //live4346A890; highest index at native byte+24
    const std::int32_t*& max_clients; //live43469300
    // Native owner+730 captures CSun, not a CEnvTonemapController class.
    void*& tonemap_client_class; //canonical CSun434684C4
    void*& smoke_registry; //live434679C0, count at+0C
};

struct WorldNetvars {
    std::uint32_t tonemap_disabled; // resolver 0x37482375
    std::uint32_t fog_controller; // resolver 0xDC91E6CF
    std::uint32_t fog_enabled; // resolver 0x60816092
};
struct SmokeRuntimeBindings {
    void* const& engine; //live4346A514
    void* const& entity_list; //live4346A890,read after engine12
    void* const& smoke_registry; //live434679C0,count+0C
};

// The captured engine uses a parent-linked ConVar and pointer-xored integer
// storage at +0x30/+0x40. No command-string substitutes are used.
struct WorldConvarOverride {
    void* variable = nullptr;
    std::uint32_t saved = 0;
    bool active = false;
    void set(std::uint32_t value); // 0x433CEF55
    void restore(); // 0x433AB872
};

class WorldRuntime {
public:
    WorldRuntime(esp::EffectsSettings& settings, WorldInterfaces& interfaces, WorldNetvars offsets,
                 const std::uint8_t& suspended)
        : settings(settings), interfaces(interfaces), offsets(offsets), suspended(suspended) {}
    esp::EffectsSettings& settings;
    WorldInterfaces& interfaces;
    WorldNetvars offsets;
    const std::uint8_t& suspended; // native 0x43468E58
    std::array<void*, 9> excluded_materials{};
    std::uint8_t material_flags = 0; // owner +0x452
    float previous_wall_alpha = 1.0f; // +0x6D4
    std::int32_t previous_prop_transparency = 100; // +0x6D8
    std::int32_t previous_prop_transparency_2 = 100; // +0x6DC
    std::uint32_t previous_night_rgb = 0; // +0x6E0
    bool fog_overridden() const noexcept {
        return (material_flags & 8) != 0;
    }
    std::uint8_t saved_fog_enabled = 0;
    WorldConvarOverride post_processing; // owner +0x6E8
    WorldConvarOverride
        cascaded_shadows; //constructor+6F4, cl_csm_shadows; no current render override
    std::array<WorldConvarOverride, 4> sky; // +0x700/+0x70C/+0x718/+0x724

    void update(); // complete 0x433B9D00
    void
    begin_render(); // world-effect range 0x433FCE36..0x433FCFC1; camera belongs to CameraRuntime
    void begin_render_for_player(void* local); //same effect block after the caller's local lookup
    void end_render(); // the five original 0x433AB872 restores
    void smoke_before_render(); // complete 0x433C08F4
    void smoke_after_render(); // complete 0x433E6305
    void smoke_before_render(const SmokeRuntimeBindings&);
    void smoke_after_render(const SmokeRuntimeBindings&);
};

// Complete 0x43359CA0. The supplied table is the engine's saved original
// vtable, whose slot 13 receives the unchanged argument when not suppressed.
bool filter_flash_effect(const esp::EffectsSettings& settings, void* receiver,
                         void** original_vtable, void* effect_argument);
void bind_flash_effect_runtime(const esp::EffectsSettings&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall flash_effect_hook(void* receiver, void* ignored_edx, void* effect_argument);
#else
bool flash_effect_hook(void* receiver, void* ignored_edx, void* effect_argument);
#endif

} // namespace skeet::effects
