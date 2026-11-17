#pragma once
#include "native_data_image.hpp"
#include <span>
#include <stdexcept>

namespace skeet::engine {
struct BuildSceneClassRecipe {
    std::uint32_t operand, class_hash, destination_cell;
    const char* network_name;
};
std::span<const BuildSceneClassRecipe, 20> build_scene_class_manifest() noexcept;
class BuildSceneClassFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 10;
    BuildSceneClassFailure(std::size_t, const BuildSceneClassRecipe&);
    const std::size_t index;
    const std::uint32_t operand, class_hash, destination_cell;
    const char* const network_name;
};

// Source scene bindings for the supported client65272025 build. Requires its
// canonical module4346A55C and VClient018434683E0. Reads the genuine virtual8
// GetAllClasses list, preserving ClientClass pointer identity independently of
// assigned class IDs. These twenty cells are not RTTI descriptors or vtables.
// Also discovers the real optional IsBreakableEntity entry43468F30 through
// verified code/RTTI operands. It calls no breakability/entity factory method.
//
// The supported build has289 registered classes. The whole390..42C class loop
// calls GetAllClasses once per row and publishes successful rows in native
// order. A missing class reaches native ZwTerminateThread(-2,10) before that
// row's destination read/store; the source failure exposes this boundary and
// preserves prior stores. The startup owner performs the terminal OS call.
// Invalid image/list provenance is an unsupported source binding. Keep the
// module loaded and run before constructing source scene views.
void initialize_build_scene_bindings(NativeDataImage&);
} // namespace skeet::engine
