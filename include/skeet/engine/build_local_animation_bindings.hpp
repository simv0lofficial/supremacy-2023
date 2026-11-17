#pragma once
#include "native_data_image.hpp"
#include "skeet/animation/local_helpers.hpp"
#include "skeet/misc/network_fix.hpp"

namespace skeet::engine {
struct BuildLocalAnimationBindings {
    animation::InitializeSequenceMapping initialize_mapping;
    animation::SelectSymbolSequence select_symbol_sequence;
    animation::LookupAnimationSymbol find_symbol, add_symbol;
    void* symbol_table;
    void* activity_vtable;
    void* layer_weight;
    misc::NetworkFixEngineEntry network_process;
    std::uintptr_t strafe_call_site, proxy_call_site, sequence_call_site, proxy_context,
        velocity_continuation;
};
// Exact supported client65272025/engine64EE7E86 build. Resolves genuine
// functions and call-site identities from checked module code/operands; it
// neither executes engine code nor loads a module. All publications occur
// after validation, including the independently resolved animation RTTI table.
// Publishes the three actual local-query caller identities434674E0/4346756C/
//43468518 after proving their virtual88/170 engine call scopes.
BuildLocalAnimationBindings initialize_build_local_animation_bindings(NativeDataImage&);
// External engine ABI only: ECX=state, EDX=0, stack(layer), XMM0 result, RET4.
animation::LocalLayerWeightEngine native_local_layer_weight_engine(void* entry);
} //namespace skeet::engine
