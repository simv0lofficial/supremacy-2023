#pragma once
#include "native_data_image.hpp"
#include "loaded_modules.hpp"
#include "pe_exports.hpp"
#include "skeet/effects/model_render.hpp"

namespace skeet::engine {
struct BuildMaterialBindings {
    NativeDataImage& data;
    effects::CreateMaterialVariable create_variable;
    void* localization;
};
// Acquire only already loaded modules. The material-variable fastcall is an
// internal materialsystem function, identified by its unique supported-build
// instruction body. Localize_001 is the actual localize.dll registration;
// its proven MOV EAX,object;RET creator is evaluated from its live operand.
// All provenance checks finish before43467A68/4346A31C are published. No game
// constructor, material allocation or captured function is executed here.
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage&,
                                                         const LoadedModuleEnvironment&,
                                                         NativeImageEnvironment);
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage&);
} //namespace skeet::engine
