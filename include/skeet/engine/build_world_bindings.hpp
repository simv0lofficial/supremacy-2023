#pragma once
#include "native_data_image.hpp"
#include "skeet/script/panorama_api.hpp"

namespace skeet::engine {
struct GameSystemRegistrationNode;
struct BuildWorldBindings {
    NativeDataImage& data;
    void* rank_context;
    void** rank_data;
    GameSystemRegistrationNode** registry_head;
    script::PanoramaBeginEngineCall begin_engine_call;
    script::PanoramaEndEngineCall end_engine_call;
};
// Supported live client/panorama instruction and data provenance. The lazy
// rank slot and registration head remain engine-owned live cells; no object
// is allocated or copied from a captured process and no game code is called.
BuildWorldBindings initialize_build_world_bindings(NativeDataImage&);
// The two live CUtlVector size members used by MapGameSystemRuntime. Verify
// the real client CRT stores and independent append consumers before binding
//43468D0C/43467A70; no list or captured endpoint is copied into source memory.
void initialize_build_map_game_system_bindings(NativeDataImage&);
} //namespace skeet::engine
