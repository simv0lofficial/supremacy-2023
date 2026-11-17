#pragma once
#include "game_system.hpp"

namespace skeet::client {
class NativeStartupSources;
}

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_MAP_SYSTEM_MEMBER __thiscall
#else
#define SKEET_MAP_SYSTEM_MEMBER
#endif

// The abstract base has13 entries, including its scalar-deleting destructor.
struct MapGameSystemBaseVtable {
    GameSystemEntry<const char*> name;
    GameSystemEntry<bool> initialize;
    GameSystemEntry<void> slot2, slot3, slot4;
    GameSystemEntry<void> map_entity_text; //43325578
    GameSystemEntry<void> slot6, slot7, slot8, slot9, slot10;
    GameSystemEntry<bool> slot11;
    GameSystemEntry<void*, std::uint8_t> destroy;
};
//4346F3B8 also includes the three IGameSystemPerFrame methods. The engine's
//client+2F2D85 calls slot14 with its frame-time argument on the stack.
struct MapGameSystemVtable {
    MapGameSystemBaseVtable base;
    GameSystemEntry<void> pre_render; //433CCE8E
    GameSystemEntry<void, float> update; //43328352
    GameSystemEntry<void> post_render; //433CCE8E
};
struct MapGameSystemEndpoint {
    const MapGameSystemBaseVtable* vtable;
};
struct MapGameSystemVector {
    MapGameSystemEndpoint** data;
    std::int32_t capacity, grow_size, size;
    MapGameSystemEndpoint** elements;
};
struct MapGameSystemOwner {
    const GameSystemEntry<void>* vtable;
    MapGameSystemVector* first;
    MapGameSystemVector* second;
    MapGameSystemEndpoint endpoint;
    MapGameSystemOwner* initialized;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(MapGameSystemBaseVtable) == 13 * 4);
static_assert(sizeof(MapGameSystemVtable) == 16 * 4);
static_assert(offsetof(MapGameSystemVtable, update) == 14 * 4);
static_assert(sizeof(MapGameSystemVector) == 0x14);
static_assert(offsetof(MapGameSystemVector, size) == 0xC);
static_assert(sizeof(MapGameSystemOwner) == 0x14);
static_assert(offsetof(MapGameSystemOwner, endpoint) == 0xC);
static_assert(offsetof(MapGameSystemOwner, initialized) == 0x10);
#endif

struct MapGameSystemBindings {
    // Actual globals contain pointers to each list's size member. They are
    // read by primary slot1; the resulting list pointers remain cached.
    std::int32_t* const& first_size; //43468D0C
    std::int32_t* const& second_size; //43467A70
    void* const& allocator; //43467204, slots1/3
    void* const& engine; //4346A514, slot99
    const OwnerRegistry& map_entity_parsers; //43475770/74/78
};

// Keep this runtime alive until all its registered owners have been destroyed.
// Source context precedes each source-owned endpoint vtable, preserving the exact
// native0x14 owner allocation and the constructor's sparse writes.
class MapGameSystemRuntime {
public:
    MapGameSystemRuntime(OwnerRegistryBinding, MapGameSystemBindings);
    MapGameSystemRuntime(const MapGameSystemRuntime&) = delete;
    MapGameSystemRuntime& operator=(const MapGameSystemRuntime&) = delete;
    MapGameSystemOwner* create(); //4348E6AA
    void initialize(MapGameSystemOwner&); //4348DAA7
    void append(MapGameSystemOwner&); //4331FED0
    void remove(MapGameSystemOwner&); //4341799E
    void destroy(MapGameSystemOwner*); //4334B488
private:
    struct PrimaryTable {
        std::array<GameSystemEntry<void>, 9> entries;
        MapGameSystemRuntime* runtime;
    } primary_;
    struct EndpointTable {
        MapGameSystemRuntime* runtime;
        MapGameSystemVtable entries;
    } endpoint_;
    struct DestroyedEndpointTable {
        MapGameSystemRuntime* runtime;
        MapGameSystemBaseVtable entries;
    } destroyed_;
    OwnerRegistryBinding owners_;
    MapGameSystemBindings bindings_;
    static MapGameSystemRuntime& runtime(MapGameSystemOwner&);
    static MapGameSystemRuntime& runtime(MapGameSystemEndpoint&);
    static void SKEET_MAP_SYSTEM_MEMBER destroy_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER initialize_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER append_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER remove_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER map_entity_text_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER frame_update_entry(void*, float);
    static void* SKEET_MAP_SYSTEM_MEMBER destroy_endpoint_entry(void*, std::uint8_t);
};
void bind_map_game_system_factory(client::NativeStartupSources&, MapGameSystemRuntime&);

#undef SKEET_MAP_SYSTEM_MEMBER
} //namespace skeet::engine
