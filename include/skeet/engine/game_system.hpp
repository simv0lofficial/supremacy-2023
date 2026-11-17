#pragma once
#include "frame_update.hpp"
#include "unload.hpp"
#include "skeet/esp/projection.hpp"
#include <array>

namespace skeet::client {
class ShutdownRuntime;
}
namespace skeet::misc {
struct DisablePureConfig;
}
namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_SYSTEM_MEMBER __thiscall
#define SKEET_SYSTEM_FAST __fastcall
#else
#define SKEET_SYSTEM_MEMBER
#define SKEET_SYSTEM_FAST
#endif

// The argument counts are part of the actual4347058C interface. In particular,
// its false/true/empty entries have different RET immediates.
template <class R, class... A> using GameSystemEntry = R(SKEET_SYSTEM_MEMBER*)(void*, A...);
struct GameSystemVtable {
    GameSystemEntry<const char*> name; //0 4332938A
    GameSystemEntry<bool> initialize; //1 43408189: false
    GameSystemEntry<void> shutdown; //2 433EF91A
    GameSystemEntry<bool, std::uintptr_t> slot3, slot4;
    GameSystemEntry<void> slot5, slot6;
    GameSystemEntry<bool, std::uintptr_t> slot7;
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t> slot8;
    GameSystemEntry<void, std::uint8_t> frame; //9 4334820E
    std::array<GameSystemEntry<void>, 8> slots10_17;
    GameSystemEntry<std::uintptr_t, std::uintptr_t> slot18, slot19;
    GameSystemEntry<void> owner_phase6, slot21, owner_phase8, slot23;
    GameSystemEntry<void> update_phase2, update_phase3; //24,25
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t, const std::int32_t*, const std::int32_t*>
        capture_camera; //26 4334331D
    GameSystemEntry<bool, float*, float*, float*> view; //27 433B91CB
    GameSystemEntry<bool, std::uintptr_t> slot28;
    GameSystemEntry<bool> slot29, slot30, slot31, slot32;
    GameSystemEntry<std::uintptr_t, std::uintptr_t> slot33;
    GameSystemEntry<void> slot34, slot35;
    GameSystemEntry<bool, std::uintptr_t, std::uintptr_t> slot36;
    GameSystemEntry<bool, std::uintptr_t, std::uintptr_t, std::uintptr_t> slot37;
    GameSystemEntry<void> update_phase0, update_phase1, slot40, slot41;
    GameSystemEntry<void, std::int32_t> before_stage, after_stage; //42,43
    GameSystemEntry<void> pure_file_override, update_phase4; //44,45
    GameSystemEntry<std::uint32_t> slot46;
};
struct GameSystemEndpoint {
    const GameSystemVtable* vtable = nullptr;
};
struct GameSystemVector {
    GameSystemEndpoint** data = nullptr;
    std::int32_t capacity = 0, grow_size = 0, size = 0;
    GameSystemEndpoint** elements = nullptr;
};
struct GameSystemManager {
    void* vtable = nullptr;
    GameSystemVector systems;
    std::array<std::uint32_t, 5> reserved{};
    std::int32_t current = -1; // native manager+2C
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(GameSystemVtable) == 47 * 4);
static_assert(offsetof(GameSystemVtable, frame) == 9 * 4);
static_assert(offsetof(GameSystemVtable, before_stage) == 42 * 4);
static_assert(sizeof(GameSystemVector) == 0x14);
static_assert(offsetof(GameSystemManager, systems) == 4);
static_assert(offsetof(GameSystemManager, current) == 0x2C);
#endif

struct GameSystemBindings {
    void* const& engine; //4346A514, slots5/37/101
    void* const& view_provider; //4346A624, slot12(-1)
    void* const& filesystem; //43467F68, slot128's actual instruction
    const misc::DisablePureConfig& disable_pure; //43475974 owner+1C,177AB88C
    float (*tangent)(float); //genuine CRT7750CA90
    float (*arctangent)(float); //genuine CRT7750A400
};
struct GameSystemCameraState {
    std::array<float, 3>& origin; //43479CCC
    std::array<float, 3>& angles; //43479CD8
    float& field_of_view; //43468E80
    float& aspect_field_of_view; //4346D548
    esp::Projection& projection; //matrix434798E0, halves43479920/434798DC
};

class GameSystemRuntime {
public:
    GameSystemRuntime(client::ShutdownRuntime&, const OwnerRegistry& updates,
                      const OwnerRegistry& frames, std::int32_t& frame_counter,
                      std::int32_t& update_stage, GameSystemBindings, GameSystemCameraState);
    client::ShutdownRuntime& shutdown;
    const OwnerRegistry& updates;
    const OwnerRegistry& frames;
    std::int32_t& frame_counter;
    std::int32_t& update_stage;
    GameSystemBindings bindings;
    GameSystemCameraState view_state;
    void capture_camera(const std::int32_t* width, const std::int32_t* height); //4334331D
    void capture_projection(); //43417FE8
    void before_stage(std::int32_t); //43419ADC
    void after_stage(std::int32_t); //4337E9DE
    void apply_pure_file_override(); //4332922E, same config as4335C233
};

// Full4348E09F owner, lifecycle43472A2C. The manager pointer is captured at
// construction; allocator is the live43467204 engine allocation interface.
struct GameSystemOwner {
    const GameSystemEntry<void>* vtable = nullptr;
    GameSystemManager* manager = nullptr;
    std::int32_t saved_index = -1;
    GameSystemEndpoint endpoint;
    GameSystemOwner* installed = nullptr;
    GameSystemRuntime* runtime = nullptr;
    void* const* allocator = nullptr;
    JobMemory memory{};
    void install(); //4347F4E5 ->433F4786/4337CF0C
    void remove(); //43331AAF ->433B9059
};
GameSystemOwner* create_game_system_owner(OwnerRegistryBinding, GameSystemManager*,
                                          void* const* allocator, GameSystemRuntime&);

// Separate constructor4347DA14: this six-entry secondary interface is linked
// through43468D50. It is not the47-entry manager interface above.
struct GameSystemRegistrationVtable {
    GameSystemEntry<void> slot0, slot1;
    GameSystemEntry<void, std::uintptr_t> slot2;
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t> dispatch;
    GameSystemEntry<void, std::uintptr_t> slot4;
    GameSystemEntry<bool> slot5;
};
struct GameSystemRegistrationEndpoint {
    const GameSystemRegistrationVtable* vtable = nullptr;
};
struct GameSystemRegistrationNode {
    const char* name = nullptr;
    GameSystemRegistrationEndpoint* endpoint = nullptr;
    GameSystemRegistrationNode* next = nullptr;
};
using RegisteredClientEntry = void(SKEET_SYSTEM_FAST*)(void*, std::uintptr_t, std::uintptr_t,
                                                       std::uintptr_t, std::uintptr_t);
struct RegisteredClientEndpoint {
    const RegisteredClientEntry* vtable = nullptr;
};
void dispatch_registered_client_callbacks(const OwnerRegistry&, std::uintptr_t, std::uintptr_t,
                                          std::uintptr_t, std::uintptr_t); //433B8C46
struct GameSystemRegistrationOwner {
    const GameSystemEntry<void>* vtable = nullptr;
    GameSystemRegistrationEndpoint endpoint;
    GameSystemRegistrationNode node;
    std::uint8_t registered = 0;
    char empty_name = 0;
    GameSystemRegistrationNode** const* head_slot = nullptr;
    const OwnerRegistry* callbacks = nullptr;
    UnloadState* unload = nullptr;
    JobMemory memory{};
    void install(); //43417897
    void remove(); //433AE857; hard detach leaves all link state untouched
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(GameSystemRegistrationVtable) == 24);
static_assert(offsetof(GameSystemOwner, endpoint) == 0xC);
static_assert(offsetof(GameSystemOwner, installed) == 0x10);
static_assert(offsetof(GameSystemRegistrationOwner, endpoint) == 4);
static_assert(offsetof(GameSystemRegistrationOwner, node) == 8);
static_assert(offsetof(GameSystemRegistrationOwner, registered) == 0x14);
static_assert(offsetof(GameSystemRegistrationOwner, empty_name) == 0x15);
#endif
GameSystemRegistrationOwner*
create_game_system_registration(OwnerRegistryBinding, GameSystemRegistrationNode** const* head_slot,
                                const OwnerRegistry& callbacks, UnloadState&);

#undef SKEET_SYSTEM_MEMBER
#undef SKEET_SYSTEM_FAST
} //namespace skeet::engine
