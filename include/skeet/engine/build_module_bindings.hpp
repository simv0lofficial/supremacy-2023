#pragma once
#include "loaded_modules.hpp"
#include "native_data_image.hpp"
#include <array>
#include <span>
#include <string_view>

namespace skeet::engine {
struct SourceInitializationImports;
struct BuildModuleRecipe {
    std::uint32_t destination_cell;
    std::uint32_t salted_name_hash;
    std::u16string_view module_name;
};

// These17 loaded PE32 modules supply all41 build interface acquisitions. The
//working dump independently matches each canonical cell to its module base.
std::span<const BuildModuleRecipe, 17> build_module_manifest() noexcept;
// The Steam overlay has no interface request in the41-row acquisition table.
// Its base is resolved separately and may be null at this publication cut.
// The later hook-descriptor phase requires it: a missing overlay reaches
// native termination12 after25 descriptors/93 slots, before operands898/89C.
const BuildModuleRecipe& build_overlay_module_recipe() noexcept;

struct BuildModuleBases {
    std::array<void*, 17> modules{};
    void* overlay = nullptr;
    void* at(std::u16string_view exact_module_name) const;
};

// Resolve only the existing PEB loader list, with the real loader-lock imports.
// Verify its exact case-folded basename and bounded MZ/PE32/I386 headers.
// Stage all17 mandatory results before changing any canonical cell. This
//publishes module bases alone: no code RVA, interface, offset or decoded helper
//pointer is obtained from a captured process. Repeated calls refresh the bases.
BuildModuleBases publish_build_module_bases(NativeDataImage&, const LoadedModuleEnvironment&);
BuildModuleBases publish_build_module_bases(NativeDataImage&, const SourceInitializationImports&);
BuildModuleBases publish_build_module_bases(NativeDataImage&);
} //namespace skeet::engine
