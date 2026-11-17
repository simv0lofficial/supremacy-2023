#pragma once
#include "native_data_image.hpp"
#include "interface_call.hpp"

namespace skeet::engine {
struct BuildHistoryBindings {
    MemberFunction<std::int32_t, std::int32_t> activity;
    MemberFunction<const char*, std::int32_t> sequence_name;
    void* sequence_displacement; //custom ECX/EDX + caller-cleaned stack ABI
};
// Actual client.dll65272025 functions independently matched to the three
// working-dump globals. Requires the canonical bound client module, verifies
// PE sections, function instructions, direct callees and every return ABI,
// then publishes4346A364/434678C8/43468DC0 together. Runs no client function.
BuildHistoryBindings initialize_build_history_bindings(NativeDataImage&);
} //namespace skeet::engine
