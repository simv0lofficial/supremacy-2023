#pragma once
#include "native_data_image.hpp"
#include "skeet/network/packet_world.hpp"

namespace skeet::engine {
// The build's4346A7CC external primitive is engine.dll's occlusion query.
// Reads loaded module bytes only; no original engine function is called here.
void initialize_build_packet_bindings(NativeDataImage&);
class PacketBoxRuntime final {
public:
    explicit PacketBoxRuntime(const NativeDataImage&);
    network::PacketBoxQuery binding() noexcept;
    bool query(const network::Bounds&, const network::Bounds&, network::Vector&, float) const;

private:
    const std::uintptr_t& entry_;
    static bool invoke(void*, const network::Bounds&, const network::Bounds&, network::Vector&,
                       float);
};
namespace detail {
bool invoke_packet_box(std::uintptr_t, const network::Bounds&, const network::Bounds&,
                       network::Vector&, float);
}
} //namespace skeet::engine
