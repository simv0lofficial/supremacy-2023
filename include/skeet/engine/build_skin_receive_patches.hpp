#pragma once
#include "native_data_image.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include <array>
#include <span>
#include <stdexcept>

namespace skeet::engine {
struct BuildSkinReceivePatchRecipe {
    std::uint32_t operand, class_hash, table_hash, property_hash, patch_address;
    const char* class_name;
    const char* table_name;
    const char* property_name;
};
// Four ordered A48..A84 recipes and the later AFC agent recipe. These are
// explicit fixed-build metadata, not an implementation of its protected getter.
std::span<const BuildSkinReceivePatchRecipe, 5> build_skin_receive_patch_manifest() noexcept;
enum class BuildSkinReceivePatchFailureReason {
    missing_property,
    missing_original,
    already_initialized
};
class BuildSkinReceivePatchFailure final : public std::runtime_error {
public:
    BuildSkinReceivePatchFailure(std::size_t, const BuildSkinReceivePatchRecipe&,
                                 BuildSkinReceivePatchFailureReason);
    const std::size_t index;
    const BuildSkinReceivePatchRecipe recipe;
    const BuildSkinReceivePatchFailureReason reason;
};
class BuildSkinReceivePatches {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    std::span<client::EntityUpdatePatch* const, 4> sequence() const noexcept {
        return sequence_;
    }
    client::EntityUpdatePatch& agent() const noexcept {
        return agent_;
    }

private:
    explicit BuildSkinReceivePatches(NativeDataImage&);
    NativeDataImage& data_;
    std::array<client::EntityUpdatePatch*, 4> sequence_;
    client::EntityUpdatePatch& agent_;
    friend BuildSkinReceivePatches initialize_build_skin_receive_patches(NativeDataImage&);
};
// Resolve genuine current RecvProp.proxy fields through VClient018 slot8 and
// the ordinary native lookup helpers. Capture their current callbacks in the
// five canonical nodes without changing a property, node.next, or list head.
// Bind these nodes to the same SkinsRuntimeOwner before publishing any list.
// All-five validation and typed rejection are explicit source policy while
// the native missing-property continuation and list-write order are unresolved.
BuildSkinReceivePatches initialize_build_skin_receive_patches(NativeDataImage&);
} //namespace skeet::engine
