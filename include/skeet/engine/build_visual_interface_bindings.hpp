#pragma once
#include "native_data_image.hpp"
#include "pe_exports.hpp"

namespace skeet::engine {
struct BuildVisualInterfaceBindings {
    void* view_provider;
    void* temp_entities;
    void* beams;
    void* weapon_system;
};
// These four genuine inline client objects are not CreateInterface entries.
// Validate the client build, exact RTTI, unique live instances and their real
// instruction operands/ABIs, then publish4346A624/434679EC/4346A898/43467B54. No game code runs
// during acquisition and no constructor is replayed over a loaded object.
BuildVisualInterfaceBindings initialize_build_visual_interface_bindings(NativeDataImage&,
                                                                        NativeImageEnvironment);
BuildVisualInterfaceBindings initialize_build_visual_interface_bindings(NativeDataImage&);
} // namespace skeet::engine
