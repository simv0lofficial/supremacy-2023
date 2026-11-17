#pragma once
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::animation {
class FixAnimationsRuntime;
enum class ReceivedLayerField {
    cycle,
    previous_cycle,
    sequence,
    weight,
    playback_rate,
    weight_delta_rate,
    order
};
class LayerReceiveRuntime {
public:
    LayerReceiveRuntime(engine::NativeDataImage& data, const engine::NativeOffsetStorage& offsets,
                        FixAnimationsRuntime& animations) noexcept
        : data_(data), offsets_(offsets), animations_(animations) {}
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    FixAnimationsRuntime& animations() const noexcept {
        return animations_;
    }
    void receive(ReceivedLayerField, const engine::RecvProxyData*, void* layer, void* output);

private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetStorage& offsets_;
    FixAnimationsRuntime& animations_;
    std::uint32_t offset(std::uint32_t, std::uint32_t, std::uint32_t) const;
};
} //namespace skeet::animation
