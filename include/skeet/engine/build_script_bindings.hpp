#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
struct BuildScriptBindings {
    NativeDataImage& data;
    const std::int32_t* old_command_ack;
    const std::int32_t* command_ack;
    void* class_size_interface;
};
// Verify the actual loaded engine/client instruction operands, client-state
// publications and CClassMap implementation before publishing the three cells.
// This inspects passive module bytes; it calls no game method or constructor.
BuildScriptBindings initialize_build_script_bindings(NativeDataImage&);
}
