#pragma once
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/offset_storage.hpp"

namespace skeet::client {
class PlayerStateCache;
// Eight complete entity receive callbacks. Saved engine callbacks, engine
// interfaces and encoded fields are reread at their original use sites.
class PlayerReceiveRuntime {
public:
    PlayerReceiveRuntime(engine::NativeDataImage& data, const engine::NativeOffsetStorage& offsets,
                         PlayerStateCache& players) noexcept
        : data_(data), offsets_(offsets), players_(players) {}
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    PlayerStateCache& player_cache() const noexcept {
        return players_;
    }
    void simulation_time(const engine::RecvProxyData*, void*, void*); //433FCA81
    void tick_base(const engine::RecvProxyData*, void*, void*); //43402A54
    void velocity(const engine::RecvProxyData*, void*, void*, unsigned component);
    void eye_angle(const engine::RecvProxyData*, void*, void*, unsigned component);
    void client_animation(const engine::RecvProxyData*, void*, void*); //4338A848
private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetStorage& offsets_;
    PlayerStateCache& players_;
    void timestamped(const engine::RecvProxyData*, void*, void*, std::uint32_t, std::size_t,
                     std::size_t);
    void original(std::uint32_t, const engine::RecvProxyData*, void*, void*) const;
};
} //namespace skeet::client
