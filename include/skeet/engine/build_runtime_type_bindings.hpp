#pragma once
#include "native_data_image.hpp"
#include "rtti.hpp"
#include <array>
#include <span>
#include <stdexcept>

namespace skeet::engine {
enum class BuildRuntimeType : std::size_t {
    trace_filter_simple,
    trace_filter_skip_two_entities,
    base_player_animation_state,
    game_rules_proxy,
    cs_ragdoll,
    cs_ragdoll_renderable,
    econ_item_attribute,
    viewmodel_attachment_renderable,
    predicted_viewmodel_renderable,
    weapon_world_model_renderable,
    econ_wearable_renderable,
    cs_player,
    base_cs_grenade,
    player_addon_model_renderable,
    weapon_cs_base,
    count
};
struct BuildRuntimeTypeRecipe {
    BuildRuntimeType identity;
    std::uint32_t operand, type_hash, destination_cell, object_offset;
    const char* decorated_name;
};
// The fifteen orderedFC0..1070 triples are explicit fixed-build data. They
// neither implement nor substitute for the protected metadata accessor.
std::span<const BuildRuntimeTypeRecipe, 15> build_runtime_type_manifest() noexcept;

enum class BuildRuntimeTypeFailureReason {
    missing_type,
    unexpected_type_name,
    missing_vtable,
    invalid_vtable
};
class BuildRuntimeTypeFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 10;
    BuildRuntimeTypeFailure(std::size_t, const BuildRuntimeTypeRecipe&,
                            BuildRuntimeTypeFailureReason);
    const std::size_t index;
    const BuildRuntimeType identity;
    const std::uint32_t operand, type_hash, destination_cell, object_offset;
    const char* const decorated_name;
    const BuildRuntimeTypeFailureReason reason;
};

class BuildRuntimeTypeBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    RttiTypeDescriptor* type(BuildRuntimeType) const;
    // Reload each actual source cell. Later native/source stores, including a
    // cleared viewmodel-attachment table, remain observable to its consumers.
    void** vtable(BuildRuntimeType) const;

private:
    BuildRuntimeTypeBindings(NativeDataImage&, const std::array<RttiTypeDescriptor*, 15>&);
    NativeDataImage& data_;
    std::array<RttiTypeDescriptor*, 15> types_;
    friend BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&,
                                                                           NativeImageEnvironment);
};

// Read the already published client.dll module cell4346A55C and resolve its
// real MSVC RTTI through433B305B/433EC4BD. No game function is called. The exact
// supported PE32 build/sections and each decorated name/table are checked.
// Successful rows publish in manifest order. Native missing-type/table paths
// reach4337F065 and ZwTerminateThread(-2,10) before the failed row's store.
// This boundary reports a typed failure, retaining earlier rows; the startup
// owner performs that terminal sequence using its actual state/imports.
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&,
                                                                NativeImageEnvironment);
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&);
} //namespace skeet::engine
