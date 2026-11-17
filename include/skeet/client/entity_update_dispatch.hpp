#pragma once
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <cstdint>

namespace skeet::ui {
class SkinsMenu;
}
namespace skeet::client {
// Native43476908 list. Exchange targets and replacement words are borrowed;
// the engine's imported InterlockedExchange performs the actual atomic write.
struct EntityUpdatePatch {
    EntityUpdatePatch* next = nullptr;
    volatile std::int32_t* target = nullptr;
    std::int32_t after = 0, before = 0;
};
struct EntityUpdateGlobals {
    std::uint32_t& phase; //43476904, set1 before and4 after
    EntityUpdatePatch*& patches; //43476908, reread for the second pass
    std::int32_t& local_team; //4346D4BC
    EntityObserverRegistry& receivers; //43475BA8/AC/B0, concrete source owners
};
struct EntityUpdateBindings {
    using ExchangeWord = std::int32_t(__stdcall*)(volatile std::int32_t*, std::int32_t);
    void* engine_client = nullptr;
    void* entity_list = nullptr;
    std::uint32_t team_offset = 0; //resolved434688B0^4346A518^8B5586F6
    ExchangeWord exchange = nullptr; //recorded kernel32!InterlockedExchange
};
#if defined(_M_IX86) || defined(__i386__)
using EntityUpdateReceiver = void(__fastcall*)(void*, void*, std::uintptr_t);
#else
using EntityUpdateReceiver = void (*)(void*, void*, std::uintptr_t);
#endif
// Legacy native-shaped constructor registration used by owners while their
// bindings migrate. The live dispatcher below uses EntityObserverRegistry.
void* register_entity_update_receiver(engine::OwnerRegistry&, void* object, engine::JobMemory);

// Complete433205A0. No speculative engine name is assigned to original slot64.
// The local player and incoming engine argument reach every captured receiver,
// including the glove43342310/mask43408846 entries, even when local is null.
void dispatch_entity_update(void* hooked_object, std::uintptr_t incoming, EntityUpdateGlobals&,
                            const EntityUpdateBindings&, ui::SkinsMenu&);
struct EntityUpdateRuntime {
    EntityUpdateGlobals globals;
    EntityUpdateBindings bindings;
    ui::SkinsMenu& skins_menu;
    void process(void* object, std::uintptr_t incoming) {
        dispatch_entity_update(object, incoming, globals, bindings, skins_menu);
    }
};
void bind_entity_update_runtime(EntityUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall entity_update_hook(void* object, void* register_edx, std::uintptr_t incoming);
#else
void entity_update_hook(void* object, void* register_edx, std::uintptr_t incoming);
#endif
} //namespace skeet::client
