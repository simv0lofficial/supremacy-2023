#pragma once
#include "hook_descriptors.hpp"
#include <array>
#include <span>

namespace skeet::animation {
class FixAnimationsRuntime;
}
namespace skeet::client {
struct CommandCreateMoveRuntime;
class PlayerCommandQueries;
class PlayerPhysicsHooks;
class LocalPlayerAnimationHooks;
class PlayerCommandHooks;
}
namespace skeet::effects {
struct EntityVisibilityRuntime;
struct FreeLookCaptureBindings;
}
namespace skeet::prediction {
struct PlayerSimulationBindings;
}

namespace skeet::engine {
enum class PlayerPrimaryKind {
    local,
    remote
};

inline constexpr std::uint32_t player_primary_module_identity = 0x4346a55c;
inline constexpr std::uint32_t player_primary_rtti_hash = 0x1b1a0d29; //.?AVC_CSPlayer@@
inline constexpr std::uint32_t player_primary_rtti_base_hash = 0;
inline constexpr std::uint32_t local_player_descriptor_identity = 0x43475954;
inline constexpr std::uint32_t remote_player_descriptor_identity = 0x43475830;
inline constexpr std::uint32_t local_player_replacement_stream = 0x6d0;
inline constexpr std::uint32_t remote_player_replacement_stream = 0xf00;

struct PlayerPrimaryNativeReplacement {
    std::uint32_t slot, native_entry, entry_operand;
};
// Captured contexts1812..1855, in the actual22-entry replacement-loop order.
inline constexpr std::array<PlayerPrimaryNativeReplacement, 22> local_player_native_replacements{
    {{78, 0x433842c1, 0x6d4},  {88, 0x4334d3eb, 0x6dc},  {109, 0x43402307, 0x6e4},
     {113, 0x4340191d, 0x6ec}, {145, 0x4333181e, 0x6f4}, {155, 0x433eb3f4, 0x6fc},
     {170, 0x433e97c1, 0x704}, {173, 0x4333c562, 0x70c}, {207, 0x4335c639, 0x714},
     {190, 0x43330f73, 0x71c}, {198, 0x43388201, 0x724}, {222, 0x43341fa7, 0x72c},
     {224, 0x43329436, 0x734}, {268, 0x433c6689, 0x73c}, {277, 0x4335b798, 0x744},
     {289, 0x4334f53d, 0x74c}, {366, 0x433c82dc, 0x754}, {294, 0x433e009a, 0x75c},
     {230, 0x4331e412, 0x764}, {337, 0x433ccea1, 0x76c}, {298, 0x433ab4b6, 0x774},
     {350, 0x433d18b0, 0x77c}}};
// Captured contexts1894..1913, in the actual10-entry replacement-loop order.
inline constexpr std::array<PlayerPrimaryNativeReplacement, 10> remote_player_native_replacements{
    {{78, 0x433abbf0, 0xf04},
     {109, 0x43402307, 0xf0c},
     {113, 0x433875be, 0xf14},
     {145, 0x4333181e, 0xf1c},
     {173, 0x4333c562, 0xf24},
     {190, 0x43330f73, 0xf2c},
     {198, 0x43388201, 0xf34},
     {194, 0x4336ee16, 0xf3c},
     {169, 0x43382222, 0xf44},
     {207, 0x4335c639, 0xf4c}}};

std::span<const PlayerPrimaryNativeReplacement>
    player_primary_native_replacements(PlayerPrimaryKind);
// Early descriptor phase: write only source entry addresses. Actual runtime
// publication belongs to PlayerHookFamily after its feature owners exist.
void bind_player_primary_static_entries(VtableHookMetadata&, PlayerPrimaryKind);

struct PlayerPrimaryHookRuntimes {
    animation::FixAnimationsRuntime& animations;
    client::CommandCreateMoveRuntime& commands;
    const prediction::PlayerSimulationBindings& simulation;
    const effects::FreeLookCaptureBindings& free_look;
    effects::EntityVisibilityRuntime& visibility;
    client::PlayerCommandQueries& queries;
    client::PlayerPhysicsHooks& physics;
    client::LocalPlayerAnimationHooks& animation_updates;
    client::PlayerCommandHooks& command_hooks;
};

// Both primary-player recipes require all concrete source owners. Per-base
// renderable/networkable/thinkable descriptors retain their separate recipes.
// All bindings, metadata and this family must remain alive until the canonical
// hook-list restoration has finished. No runtime or global state is copied.
class PlayerHookFamily {
public:
    PlayerHookFamily(VtableHookMetadata& local, VtableHookMetadata& remote,
                     VtableHookRegistry& hooks, std::byte* const& client_module,
                     NativeImageEnvironment images, HookDescriptorSystem system,
                     PlayerPrimaryHookRuntimes runtimes) noexcept;
    PlayerHookFamily(const PlayerHookFamily&) = delete;
    PlayerHookFamily& operator=(const PlayerHookFamily&) = delete;

    // Full source recipe: genuine RTTI resolution and descriptor cloning,
    // followed by every captured22/10 source replacement in its native order.
    // Do not run a preserved native replacement loop after this operation.
    void prepare(PlayerPrimaryKind);
    // Also used after an externally completed descriptor preparation. The
    // whole mapping is repeated immediately before the single install below.
    void bind_replacements(PlayerPrimaryKind);
    VtableHookNode* install(PlayerPrimaryKind, void* actual_player);
    VtableHookMetadata& metadata(PlayerPrimaryKind) const;
    // The receiver must already have been restored by the canonical hook list.
    void clear(PlayerPrimaryKind);

private:
    VtableHookMetadata& local_;
    VtableHookMetadata& remote_;
    VtableHookRegistry& hooks_;
    std::byte* const& client_module_;
    NativeImageEnvironment images_;
    HookDescriptorSystem system_;
    PlayerPrimaryHookRuntimes runtimes_;
};
} // namespace skeet::engine
