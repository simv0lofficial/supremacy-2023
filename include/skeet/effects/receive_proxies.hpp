#pragma once
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/engine/offset_storage.hpp"
#include <span>
#include <stdexcept>

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::esp {
struct EffectsSettings;
}
namespace skeet::effects {
struct VisualReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const VisualReceiveDescriptor, 5> visual_receive_descriptors() noexcept;
enum class VisualReceivePatchFailureReason {
    missing_property,
    missing_original,
    already_initialized
};
class VisualReceivePatchFailure final : public std::runtime_error {
public:
    VisualReceivePatchFailure(std::size_t, const VisualReceiveDescriptor&,
                              VisualReceivePatchFailureReason);
    const std::size_t index;
    const std::uint32_t native_entry, patch_address;
    const VisualReceivePatchFailureReason reason;
};
// Keep this lifetime through receive-patch restoration. All settings, offsets,
// player records, interfaces and saved callbacks are the shared source owners.
class VisualReceiveRuntime {
public:
    VisualReceiveRuntime(engine::NativeDataImage&, const esp::EffectsSettings&,
                         const engine::NativeOffsetStorage&, client::PlayerStateCache&);
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    void receive_smoke(const engine::RecvProxyData*, void*, void*); //4337EDF0
    void receive_flash(const engine::RecvProxyData*, void*, void*); //4338A0C3
    void receive_observer(const engine::RecvProxyData*, void*, void*); //4340C7B4
    void receive_fov(const engine::RecvProxyData*, void*, void*); //4339EBD4
    void receive_sun(const engine::RecvProxyData*, void*, void*); //433E978B
    // The source producer must already have captured actual target/original
    // fields in these exact five canonical nodes. This publishes source entry
    // words only, leaving every link, target and original callback untouched.
    void bind_receive_patches();
    // Resolve/capture the five genuine current RecvProp slots first, then bind
    // these source receivers. No property or list is installed. All-five
    // validation is explicit source policy while the native missing-property
    // branch and list-write order remain under original-code verification.
    void initialize_receive_patches();

private:
    engine::NativeDataImage& data_;
    const esp::EffectsSettings& settings_;
    const engine::NativeOffsetStorage& offsets_;
    client::PlayerStateCache& players_;
    std::uint32_t offset(std::uint32_t, std::uint32_t, std::uint32_t) const;
    void original(std::uint32_t, const engine::RecvProxyData*, void*, void*) const;
    void capture(const engine::RecvProxyData*, void*, bool);
};
} //namespace skeet::effects
